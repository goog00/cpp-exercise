; gemv_kernel.asm
; 实现 GEMV 的汇编版本，针对 Intel Core i7-9750H (AVX2 支持)
; 计算 y = alpha * A * x + beta * y

section .text
global gemv_kernel  ; 全局符号，供 C++ 调用（无下划线，适配 Apple Clang）

gemv_kernel:
    ; 函数参数 (macOS x86-64 ABI):
    ; rdi: A (float*) - 矩阵 A 的首地址
    ; rsi: x (float*) - 输入向量 x 的首地址
    ; rdx: y (float*) - 输出向量 y 的首地址
    ; rcx: m (int)    - 矩阵行数
    ; r8:  n (int)    - 矩阵列数
    ; xmm0: alpha (float) - 标量 alpha
    ; xmm1: beta  (float) - 标量 beta

    push rbp          ; 保存调用者的基指针
    mov rbp, rsp      ; 设置栈帧

    ; 广播标量到 256 位寄存器
    vbroadcastss ymm0, xmm0  ; 将 alpha 广播到 ymm0 的 8 个单精度槽
    vbroadcastss ymm1, xmm1  ; 将 beta 广播到 ymm1 的 8 个单精度槽
    xor r9, r9               ; r9 = i，行计数器，初始化为 0

outer_loop:
    cmp r9, rcx       ; 比较 i 和 m
    jge done          ; 如果 i >= m，跳转到结束

    vxorps ymm2, ymm2, ymm2  ; 清零累加器 ymm2，用于存储点积结果
    xor r10, r10             ; r10 = j，列计数器，初始化为 0

inner_loop:
    cmp r10, r8       ; 比较 j 和 n
    jge inner_done    ; 如果 j >= n，结束内层循环

    ; 加载并计算 8 个元素
    vmovups ymm3, [rdi + r10*4]  ; 加载 A[i][j:j+8] 到 ymm3（未对齐）
    vmovups ymm4, [rsi + r10*4]  ; 加载 x[j:j+8] 到 ymm4（未对齐）
    ; FMA 指令：ymm2 += ymm3 * ymm4，累加到结果中
    vfmadd231ps ymm2, ymm3, ymm4

    add r10, 8        ; j += 8，步长为 8（处理 8 个元素）
    jmp inner_loop    ; 继续内层循环

inner_done:
    ; 水平加和：将 ymm2 的 8 个元素求和到一个标量
    vextractf128 xmm3, ymm2, 1  ; 提取高 128 位到 xmm3
    vaddps xmm2, xmm2, xmm3     ; 低 128 位和高 128 位相加
    vpermilps xmm3, xmm2, 0x0E  ; 重排：将 4 个元素中的后 2 个移到前
    vaddps xmm2, xmm2, xmm3     ; 加到前 2 个
    vpermilps xmm3, xmm2, 0x01  ; 重排：交换第 1 和第 2 个元素
    vaddps xmm2, xmm2, xmm3     ; 再相加
    vhaddps xmm2, xmm2, xmm2    ; 最后水平加和，得到标量结果

    ; 应用 alpha 和 beta
    vmulss xmm2, xmm2, xmm0      ; total *= alpha
    vmovss xmm3, [rdx + r9*4]    ; 加载 y[i] 到 xmm3
    ; FMA 指令：total += beta * y[i]
    vfmadd231ss xmm2, xmm3, xmm1
    vmovss [rdx + r9*4], xmm2    ; 存储结果回 y[i]

    inc r9                  ; i++，下一行
    lea rdi, [rdi + r8*4]   ; 更新 A 的指针到下一行（r8*4 字节）
    jmp outer_loop          ; 继续外层循环

done:
    mov rsp, rbp      ; 恢复栈指针
    pop rbp           ; 恢复基指针
    ret               ; 返回