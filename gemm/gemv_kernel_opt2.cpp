#include <immintrin.h>  // AVX2 intrinsics
#include <cstdlib>      // aligned_alloc, free
#include <cstdint>      // uintptr_t

// 对应gemv_kernel_opt2.asm的实现

// GEMV 实现：y = alpha * A * x + beta * y
// 参数与汇编版本一致，假设输入数据 32 字节对齐
void gemv_kernel(float* A, float* x, float* y, int m, int n, float alpha, float beta) {
    constexpr int BLOCK_SIZE = 256;  // 分块大小，与汇编中的 rbx = 256 一致
    constexpr int VECTOR_SIZE = 8;   // 每个 256 位向量处理 8 个 float
    constexpr int UNROLL_FACTOR = 4; // 一次处理 32 个元素 (4 个向量)

    // 广播 alpha 和 beta 到 256 位向量寄存器
    __m256 alpha_vec = _mm256_set1_ps(alpha);  // 类似 vbroadcastss ymm0, xmm0
    __m256 beta_vec = _mm256_set1_ps(beta);    // 类似 vbroadcastss ymm1, xmm1

    // 外层循环：遍历矩阵的每一行
    for (int i = 0; i < m; ++i) {
        // 初始化累加器
        __m256 sum1 = _mm256_setzero_ps();  // 累加器 1，类似 ymm2
        __m256 sum2 = _mm256_setzero_ps();  // 累加器 2，类似 ymm3
        __m256 sum3 = _mm256_setzero_ps();  // 累加器 3，类似 ymm4
        __m256 sum4 = _mm256_setzero_ps();  // 累加器 4，类似 ymm5

        // 分块循环：按列分块处理
        for (int j = 0; j < n; j += BLOCK_SIZE) {
            // 计算当前块的实际大小
            int block_end = j + BLOCK_SIZE;
            if (block_end > n) block_end = n;
            int remaining = block_end - j;  // 类似 rax = min(n - j, BLOCK_SIZE)

            // 内层循环：处理 32 个元素
            int k = j;
            for (; k + UNROLL_FACTOR * VECTOR_SIZE - 1 < j + remaining; k += UNROLL_FACTOR * VECTOR_SIZE) {
                // 加载并计算 32 个元素 (4 个 256 位向量)
                __m256 a1 = _mm256_load_ps(&A[i * n + k]);              // 类似 vmovaps ymm6, [rdi + r10*4]
                __m256 x1 = _mm256_load_ps(&x[k]);                      // 类似 vmovaps ymm7, [rsi + r10*4]
                sum1 = _mm256_fmadd_ps(a1, x1, sum1);                   // 类似 vfmadd231ps ymm2, ymm6, ymm7

                __m256 a2 = _mm256_load_ps(&A[i * n + k + VECTOR_SIZE]); // 类似 vmovaps ymm8, [rdi + r10*4 + 32]
                __m256 x2 = _mm256_load_ps(&x[k + VECTOR_SIZE]);         // 类似 vmovaps ymm9, [rsi + r10*4 + 32]
                sum2 = _mm256_fmadd_ps(a2, x2, sum2);                   // 类似 vfmadd231ps ymm3, ymm8, ymm9

                __m256 a3 = _mm256_load_ps(&A[i * n + k + 2 * VECTOR_SIZE]); // 类似 vmovaps ymm10
                __m256 x3 = _mm256_load_ps(&x[k + 2 * VECTOR_SIZE]);         // 类似 vmovaps ymm11
                sum3 = _mm256_fmadd_ps(a3, x3, sum3);                       // 类似 vfmadd231ps ymm4

                __m256 a4 = _mm256_load_ps(&A[i * n + k + 3 * VECTOR_SIZE]); // 类似 vmovaps ymm12
                __m256 x4 = _mm256_load_ps(&x[k + 3 * VECTOR_SIZE]);         // 类似 vmovaps ymm13
                sum4 = _mm256_fmadd_ps(a4, x4, sum4);                       // 类似 vfmadd231ps ymm5
            }

            // 清理循环：处理剩余元素（不足 32 个但至少 8 个）
            for (; k + VECTOR_SIZE - 1 < j + remaining; k += VECTOR_SIZE) {
                __m256 a = _mm256_load_ps(&A[i * n + k]);  // 类似 vmovaps ymm6
                __m256 x_vec = _mm256_load_ps(&x[k]);      // 类似 vmovaps ymm7
                sum1 = _mm256_fmadd_ps(a, x_vec, sum1);    // 类似 vfmadd231ps ymm2
            }

            // 注意：汇编版忽略了剩余 < 8 的元素，这里也保持一致
            // 若需完全正确性，可添加标量循环处理剩余元素
        }

        // 合并累加器
        sum1 = _mm256_add_ps(sum1, sum2);  // 类似 vaddps ymm2, ymm2, ymm3
        sum3 = _mm256_add_ps(sum3, sum4);  // 类似 vaddps ymm4, ymm4, ymm5
        sum1 = _mm256_add_ps(sum1, sum3);  // 类似 vaddps ymm2, ymm2, ymm4

        // 水平加和：将 8 个元素规约到标量
        __m128 high = _mm256_extractf128_ps(sum1, 1);  // 类似 vextractf128 xmm3, ymm2, 1
        __m128 low = _mm256_castps256_ps128(sum1);     // 低 128 位
        __m128 sum = _mm_add_ps(low, high);            // 类似 vaddps xmm2, xmm2, xmm3
        sum = _mm_hadd_ps(sum, sum);                   // 类似 vhaddps xmm2, xmm2, xmm2
        sum = _mm_hadd_ps(sum, sum);                   // 再次水平加和

        // 应用 alpha 和 beta
        float total = _mm_cvtss_f32(sum);              // 提取标量结果
        total *= alpha;                                // 类似 vmulss xmm2, xmm2, xmm0
        total = total + beta * y[i];                   // 类似 vfmadd231ss (这里用标量计算)
        y[i] = total;                                  // 存储结果
    }
}

// 测试代码，与汇编版一致
#include <iostream>
#include <vector>
#include <chrono>
#include <random>

void gemv_wrapper(float alpha, const std::vector<float>& A, const std::vector<float>& x,
                  float beta, std::vector<float>& y, int m, int n) {
    gemv_kernel(const_cast<float*>(A.data()), const_cast<float*>(x.data()), y.data(), m, n, alpha, beta);
}

void test_gemv(void (*gemv_func)(float, const std::vector<float>&, const std::vector<float>&,
                                 float, std::vector<float>&, int, int),
               const std::string& name) {
    int m = 1024, n = 1024;
    float* A_raw = static_cast<float*>(aligned_alloc(32, m * n * sizeof(float)));
    if (!A_raw) { std::cerr << "Failed to allocate A_raw\n"; return; }
    float* x_raw = static_cast<float*>(aligned_alloc(32, n * sizeof(float)));
    if (!x_raw) { std::cerr << "Failed to allocate x_raw\n"; free(A_raw); return; }
    float* y_raw = static_cast<float*>(aligned_alloc(32, m * sizeof(float)));
    if (!y_raw) { std::cerr << "Failed to allocate y_raw\n"; free(A_raw); free(x_raw); return; }

    std::vector<float> A(A_raw, A_raw + m * n);
    std::vector<float> x(x_raw, x_raw + n);
    std::vector<float> y(y_raw, y_raw + m);
    float alpha = 1.0f, beta = 0.0f;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    for (int i = 0; i < m * n; ++i) A[i] = static_cast<float>(dis(gen));
    for (int i = 0; i < n; ++i) x[i] = static_cast<float>(dis(gen));
    for (int i = 0; i < m; ++i) y[i] = static_cast<float>(dis(gen));

    auto start = std::chrono::high_resolution_clock::now();
    gemv_func(alpha, A, x, beta, y, m, n);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << name << " took " << duration.count() << " microseconds\n";

    std::cout << "First few elements of y: ";
    for (int i = 0; i < std::min(5, m); ++i) std::cout << y[i] << " ";
    std::cout << "\n";

    free(A_raw);
    free(x_raw);
    free(y_raw);
}

int main() {
    test_gemv(gemv_wrapper, "C++ AVX2 GEMV (32 elements, blocked)");
    return 0;
}