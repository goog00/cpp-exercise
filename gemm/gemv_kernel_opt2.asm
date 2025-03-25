; gemv_kernel.asm
; 优化的 GEMV 实现，针对 Intel Core i7-9750H (AVX2, FMA)
; 功能：计算 y = alpha * A * x + beta * y
; 输入参数 (macOS x86-64 ABI):
;   - rdi: A (float*) - 矩阵 A 的首地址 (m × n，行优先存储，假设 32 字节对齐)
;   - rsi: x (float*) - 输入向量 x 的首地址 (长度 n，假设 32 字节对齐)
;   - rdx: y (float*) - 输出向量 y 的首地址 (长度 m，假设 32 字节对齐)
;   - rcx: m (int)    - 矩阵 A 的行数
;   - r8:  n (int)    - 矩阵 A 的列数 (也是向量 x 的长度)
;   - xmm0: alpha (float) - 标量 alpha
;   - xmm1: beta  (float) - 标量 beta
; 优化特性：
;   - 使用 AVX2 的 256 位寄存器，一次处理 32 个元素 (4 个 256 位向量)
;   - 分块处理矩阵 A，每块 256 列，优化 L1/L2 缓存利用率
;   - 使用对齐加载 (vmovaps) 和融合乘加 (vfmadd231ps) 提高性能
;   - 包含边界检查，避免段错误

section .text
global _gemv_kernel                  ; 全局符号，无下划线，与 C++ 中的 extern "C" gemv_kernel 匹配

_gemv_kernel:
    ; 设置栈帧，保存调用者寄存器
    push rbp                        ; 保存调用者的基指针
    mov rbp, rsp                    ; 设置当前栈帧基址
    push rbx                        ; 保存 rbx (非 volatile 寄存器，用于存储块大小)

    ; 广播标量到 256 位寄存器
    vbroadcastss ymm0, xmm0         ; 将 alpha 从 xmm0 广播到 ymm0 的 8 个单精度槽
    vbroadcastss ymm1, xmm1         ; 将 beta 从 xmm1 广播到 ymm1 的 8 个单精度槽

    ; 初始化外层循环计数器
    xor r9, r9                      ; r9 = i，行索引，从 0 开始

    ; 设置缓存分块大小
    mov rbx, 256                    ; rbx = block_size，每块处理 256 列
                                    ; 256 * 4 bytes = 1KB，适配 L1 缓存 (32KB)

outer_loop:
    ; 检查是否完成所有行
    cmp r9, rcx                     ; 比较 i 和 m (行数)
    jge done                        ; 如果 i >= m，跳到结束

    ; 初始化 4 个累加器，用于块内计算
    vxorps ymm2, ymm2, ymm2         ; 累加器 1，清零，用于 A[i][j:j+8] * x[j:j+8]
    vxorps ymm3, ymm3, ymm3         ; 累加器 2，清零，用于 A[i][j+8:j+16] * x[j+8:j+16]
    vxorps ymm4, ymm4, ymm4         ; 累加器 3，清零，用于 A[i][j+16:j+24] * x[j+16:j+24]
    vxorps ymm5, ymm5, ymm5         ; 累加器 4，清零，用于 A[i][j+24:j+32] * x[j+24:j+32]
    xor r10, r10                    ; r10 = j，列索引（块内偏移），从 0 开始

block_loop:
    ; 检查是否完成当前行的所有列
    cmp r10, r8                     ; 比较 j 和 n (总列数)
    jge block_done                  ; 如果 j >= n，结束当前行处理

    ; 计算当前块的大小
    mov rax, r8                     ; rax = n，总列数
    sub rax, r10                    ; rax = n - j，剩余列数
    cmp rax, rbx                    ; 剩余列数 >= block_size?
    jle no_adjust                   ; 如果剩余 <= block_size，直接使用剩余列数
    mov rax, rbx                    ; 否则，rax = block_size，限制块大小为 256
no_adjust:                          ; rax 现在是当前块的实际大小 (min(remaining, block_size))

inner_loop:
    ; 检查是否能处理完整的 32 个元素
    cmp rax, 32                     ; 剩余列数 >= 32?
    jl cleanup_loop                 ; 如果不足 32，跳转到清理循环

    ; 处理 32 个元素，使用 4 个 256 位寄存器并行计算
    vmovaps ymm6, [rdi + r10*4]     ; 加载 A[i][j:j+8] 到 ymm6 (32 字节对齐)
    vmovaps ymm7, [rsi + r10*4]     ; 加载 x[j:j+8] 到 ymm7
    vfmadd231ps ymm2, ymm6, ymm7    ; ymm2 += A[i][j:j+8] * x[j:j+8] (融合乘加)

    vmovaps ymm8, [rdi + r10*4 + 32] ; 加载 A[i][j+8:j+16] 到 ymm8
    vmovaps ymm9, [rsi + r10*4 + 32] ; 加载 x[j+8:j+16] 到 ymm9
    vfmadd231ps ymm3, ymm8, ymm9     ; ymm3 += A[i][j+8:j+16] * x[j+8:j+16]

    vmovaps ymm10, [rdi + r10*4 + 64] ; 加载 A[i][j+16:j+24] 到 ymm10
    vmovaps ymm11, [rsi + r10*4 + 64] ; 加载 x[j+16:j+24] 到 ymm11
    vfmadd231ps ymm4, ymm10, ymm11    ; ymm4 += A[i][j+16:j+24] * x[j+16:j+24]

    vmovaps ymm12, [rdi + r10*4 + 96] ; 加载 A[i][j+24:j+32] 到 ymm12
    vmovaps ymm13, [rsi + r10*4 + 96] ; 加载 x[j+24:j+32] 到 ymm13
    vfmadd231ps ymm5, ymm12, ymm13    ; ymm5 += A[i][j+24:j+32] * x[j+24:j+32]

    ; 更新索引和剩余计数
    add r10, 32                     ; j += 32，步进 32 个元素
    sub rax, 32                     ; 剩余列数 -= 32
    jmp inner_loop                  ; 继续内层循环

cleanup_loop:
    ; 处理块内剩余元素（不足 32 个但至少 8 个）
    cmp rax, 8                      ; 剩余列数 >= 8?
    jl reduce                       ; 如果不足 8，跳到规约阶段

    ; 处理 8 个元素（1 个 256 位向量）
    vmovaps ymm6, [rdi + r10*4]     ; 加载 A[i][j:j+8] 到 ymm6
    vmovaps ymm7, [rsi + r10*4]     ; 加载 x[j:j+8] 到 ymm7
    vfmadd231ps ymm2, ymm6, ymm7    ; ymm2 += A[i][j:j+8] * x[j:j+8]
    add r10, 8                      ; j += 8
    sub rax, 8                      ; 剩余列数 -= 8
    jmp cleanup_loop                ; 继续清理循环

reduce:
    ; 合并 4 个累加器到单个向量
    vaddps ymm2, ymm2, ymm3         ; ymm2 += ymm3 (合并第 1 和第 2 个累加器)
    vaddps ymm4, ymm4, ymm5         ; ymm4 += ymm5 (合并第 3 和第 4 个累加器)
    vaddps ymm2, ymm2, ymm4         ; ymm2 += ymm4 (合并所有累加器，结果在 ymm2)

    ; 水平加和：将 ymm2 的 8 个元素规约到单个标量
    vextractf128 xmm3, ymm2, 1      ; 提取 ymm2 的高 128 位到 xmm3
    vaddps xmm2, xmm2, xmm3         ; 低 128 位 + 高 128 位，结果在 xmm2 (4 个元素)
    vhaddps xmm2, xmm2, xmm2        ; 水平加和：4 个元素 -> 2 个
    vhaddps xmm2, xmm2, xmm2        ; 再加和：2 个元素 -> 1 个，结果在 xmm2 的低位

    ; 应用 alpha 和 beta，更新 y[i]
    vmulss xmm2, xmm2, xmm0         ; total *= alpha
    vmovss xmm3, [rdx + r9*4]       ; 加载 y[i] 到 xmm3
    vfmadd231ss xmm2, xmm3, xmm1    ; total += beta * y[i] (单精度融合乘加)
    vmovss [rdx + r9*4], xmm2       ; 存储结果回 y[i]

    ; 检查是否还有更多块需要处理
    cmp r10, r8                     ; j < n?
    jl block_loop                   ; 如果未完成所有列，跳转到下一块

block_done:
    ; 移动到下一行
    inc r9                          ; i++，增加行索引
    lea rdi, [rdi + r8*4]           ; 更新 A 指针到下一行 (偏移 n*4 字节)
    jmp outer_loop                  ; 继续外层循环

done:
    ; 清理并返回
    vzeroupper                      ; 清空高位 AVX 寄存器，避免状态切换开销
    pop rbx                         ; 恢复 rbx
    mov rsp, rbp                    ; 恢复栈指针
    pop rbp                         ; 恢复基指针
    ret                             ; 函数返回