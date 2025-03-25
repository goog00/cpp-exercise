; gemv_kernel.asm
; 优化后的 GEMV 实现，针对 Intel Core i7-9750H (AVX2, FMA)
; 功能：计算矩阵-向量乘法 y = alpha * A * x + beta * y
;       其中 A 为 m × n 矩阵，x 为长度 n 的向量，y 为长度 m 的向量
;
; 输入参数 (macOS x86-64 ABI):
;   - rdi: A (float*) - 矩阵 A 的首地址
;       - 类型：单精度浮点数组 (float[])
;       - 大小：m × n 个元素，行优先存储 (row-major order)
;       - 对齐：假设 32 字节对齐 (256 位 AVX2 寄存器要求)
;       - 描述：矩阵 A 的连续内存块，A[i][j] 位于地址 A + (i * n + j) * 4
;   - rsi: x (float*) - 输入向量 x 的首地址
;       - 类型：单精度浮点数组 (float[])
;       - 大小：n 个元素
;       - 对齐：假设 32 字节对齐
;       - 描述：输入向量 x，长度与矩阵 A 的列数一致
;   - rdx: y (float*) - 输出向量 y 的首地址
;       - 类型：单精度浮点数组 (float[])
;       - 大小：m 个元素
;       - 对齐：假设 32 字节对齐
;       - 描述：输出向量 y，长度与矩阵 A 的行数一致，计算后存储结果
;   - rcx: m (int)    - 矩阵 A 的行数
;       - 类型：64 位整数
;       - 描述：表示矩阵 A 的行数，也是向量 y 的长度
;   - r8:  n (int)    - 矩阵 A 的列数
;       - 类型：64 位整数
;       - 描述：表示矩阵 A 的列数，也是向量 x 的长度
;   - xmm0: alpha (float) - 标量 alpha
;       - 类型：单精度浮点数
;       - 描述：矩阵-向量乘法的缩放因子
;   - xmm1: beta  (float) - 标量 beta
;       - 类型：单精度浮点数
;       - 描述：输出向量 y 的缩放因子
;
; 输出：
;   - y (通过 rdx 指针修改)：更新为 alpha * A * x + beta * y 的结果
;
; 优化信息：
;   - 向量处理：使用 AVX2 的 256 位寄存器 (ymm)，一次处理 16 个元素 (2 个 256 位向量)
;     - 每次循环加载 2 个 8 元素向量 (A[i][j:j+8] 和 A[i][j+8:j+16])
;     - 使用 2 个累加器 (ymm2, ymm3) 并行计算
;   - 对齐加载：使用 vmovaps 指令，要求输入数据 32 字节对齐，提升内存访问效率
;   - 融合乘加：使用 vfmadd231ps 指令，减少指令数并提高浮点运算吞吐量
;   - 循环展开：内层循环展开为 16 元素块，减少分支开销
;   - 边界处理：包含清理循环，处理剩余元素 (不足 16 个但至少 8 个)
;   - 目标硬件：Intel Core i7-9750H
;     - 支持 AVX2 (256 位向量)
;     - 支持 FMA (融合乘加)
;     - 6 核 12 线程 (当前实现为单线程，可扩展为多线程)

section .text
global _gemv_kernel                 ; 全局符号，带下划线，匹配 macOS C 命名约定

_gemv_kernel:
    ; 设置栈帧，保存调用者寄存器
    push rbp                        ; 保存调用者的基指针
    mov rbp, rsp                    ; 设置当前栈帧基址

    ; 广播标量到 256 位寄存器
    vbroadcastss ymm0, xmm0         ; 将 alpha 从 xmm0 广播到 ymm0 的 8 个单精度槽
    vbroadcastss ymm1, xmm1         ; 将 beta 从 xmm1 广播到 ymm1 的 8 个单精度槽

    ; 初始化外层循环计数器
    xor r9, r9                      ; r9 = i，行索引，从 0 开始

outer_loop:
    ; 检查是否完成所有行
    cmp r9, rcx                     ; 比较 i 和 m (行数)
    jge done                        ; 如果 i >= m，跳到结束

    ; 初始化累加器，用于计算一行内的 A * x
    vxorps ymm2, ymm2, ymm2         ; 第一个累加器，清零，用于 A[i][j:j+8] * x[j:j+8]
    vxorps ymm3, ymm3, ymm3         ; 第二个累加器，清零，用于 A[i][j+8:j+16] * x[j+8:j+16]
    xor r10, r10                    ; r10 = j，列索引，从 0 开始

inner_loop:
    ; 检查是否完成当前行的所有列
    cmp r10, r8                     ; 比较 j 和 n (总列数)
    jge inner_done                  ; 如果 j >= n，结束内层循环

    ; 计算剩余元素数并检查是否足够处理 16 个元素
    sub r8, r10                     ; r8 = n - j，临时存储剩余列数
    cmp r8, 16                      ; 剩余列数 >= 16?
    jl cleanup_loop                 ; 如果不足 16，跳转到清理循环

    ; 主循环：处理 16 个元素 (2 个 256 位向量)
    ; 处理前 8 个元素
    vmovaps ymm4, [rdi + r10*4]     ; 加载 A[i][j:j+8] 到 ymm4 (32 字节对齐)
    vmovaps ymm5, [rsi + r10*4]     ; 加载 x[j:j+8] 到 ymm5
    vfmadd231ps ymm2, ymm4, ymm5    ; ymm2 += A[i][j:j+8] * x[j:j+8] (融合乘加)

    ; 处理后 8 个元素
    vmovaps ymm6, [rdi + r10*4 + 32] ; 加载 A[i][j+8:j+16] 到 ymm6
    vmovaps ymm7, [rsi + r10*4 + 32] ; 加载 x[j+8:j+16] 到 ymm7
    vfmadd231ps ymm3, ymm6, ymm7     ; ymm3 += A[i][j+8:j+16] * x[j+8:j+16]

    ; 更新列索引
    add r10, 16                     ; j += 16，步进 16 个元素
    jmp inner_loop                  ; 继续内层循环

cleanup_loop:
    ; 清理循环：处理剩余元素 (不足 16 个)
    add r8, r10                     ; 恢复 r8 为 n (总列数)
    cmp r10, r8                     ; 比较 j 和 n
    jge reduce                      ; 如果 j >= n，跳到规约阶段

    ; 处理剩余的 8 个元素
    vmovaps ymm4, [rdi + r10*4]     ; 加载 A[i][j:j+8] 到 ymm4
    vmovaps ymm5, [rsi + r10*4]     ; 加载 x[j:j+8] 到 ymm5
    vfmadd231ps ymm2, ymm4, ymm5    ; ymm2 += A[i][j:j+8] * x[j:j+8]
    add r10, 8                      ; j += 8
    jmp cleanup_loop                ; 继续清理循环

reduce:
    ; 合并两个累加器
    vaddps ymm2, ymm2, ymm3         ; ymm2 += ymm3，合并所有累加结果到 ymm2

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

    ; 移动到下一行
    inc r9                          ; i++，增加行索引
    lea rdi, [rdi + r8*4]           ; 更新 A 指针到下一行 (偏移 n*4 字节)
    jmp outer_loop                  ; 继续外层循环

inner_done:
    ; 内层循环结束，跳转到规约阶段
    jmp reduce                      ; 直接跳到规约，避免重复代码

done:
    ; 清理并返回
    vzeroupper                      ; 清空高位 AVX 寄存器，避免状态切换开销
    mov rsp, rbp                    ; 恢复栈指针
    pop rbp                         ; 恢复基指针
    ret                             ; 函数返回