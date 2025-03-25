#include <iostream>         // 用于标准输入输出
#include <vector>           // 用于动态数组（矩阵和向量存储）
#include <immintrin.h>      // AVX2 指令集的 intrinsics 函数
#include <chrono>           // 用于性能计时
#include <random>           // 用于生成随机测试数据

// GEMV 函数声明：计算 y = alpha * A * x + beta * y
// alpha, beta: 标量系数
// A: m×n 矩阵
// x: n×1 输入向量
// y: m×1 输出向量
// m, n: 矩阵维度
void gemv(float alpha, const std::vector<float>& A, const std::vector<float>& x,
          float beta, std::vector<float>& y, int m, int n);

// 朴素实现：使用基本的双重循环计算 GEMV
void gemv_naive(float alpha, const std::vector<float>& A, const std::vector<float>& x,
                float beta, std::vector<float>& y, int m, int n) {
    // 外层循环：遍历矩阵 A 的每一行
    for (int i = 0; i < m; ++i) {
        float sum = 0.0f;  // 临时变量，存储 A[i,:] * x 的点积结果
        // 内层循环：计算第 i 行与向量 x 的点积
        for (int j = 0; j < n; ++j) {
            sum += A[i * n + j] * x[j];  // A[i][j] * x[j]，逐元素相乘并累加
        }
        // 更新 y[i]：应用 alpha 和 beta
        y[i] = alpha * sum + beta * y[i];
    }
}

// 优化 1：循环展开实现
// 通过展开内层循环，减少分支判断，提升性能
void gemv_unrolled(float alpha, const std::vector<float>& A, const std::vector<float>& x,
                   float beta, std::vector<float>& y, int m, int n) {
    for (int i = 0; i < m; ++i) {
        float sum = 0.0f;  // 点积累加器
        int j = 0;
        // 主循环：每次处理 4 个元素，减少循环次数
        for (; j <= n - 4; j += 4) {
            sum += A[i * n + j] * x[j];        // 第一个元素
            sum += A[i * n + j + 1] * x[j + 1]; // 第二个元素
            sum += A[i * n + j + 2] * x[j + 2]; // 第三个元素
            sum += A[i * n + j + 3] * x[j + 3]; // 第四个元素
        }
        // 清理循环：处理剩余元素（n 不是 4 的倍数时）
        for (; j < n; ++j) {
            sum += A[i * n + j] * x[j];
        }
        y[i] = alpha * sum + beta * y[i];  // 更新结果
    }
}

// 优化 2：AVX 向量化实现
// 使用 AVX2 的 256 位寄存器，一次处理 8 个单精度浮点数
void gemv_avx(float alpha, const std::vector<float>& A, const std::vector<float>& x,
              float beta, std::vector<float>& y, int m, int n) {
    __m256 alpha_vec = _mm256_set1_ps(alpha);  // 将 alpha 广播到 256 位向量
    __m256 beta_vec = _mm256_set1_ps(beta);    // 将 beta 广播到 256 位向量

    for (int i = 0; i < m; ++i) {
        __m256 sum = _mm256_setzero_ps();  // 初始化 256 位累加器为 0
        int j = 0;
        // 主循环：每次处理 8 个元素，利用 SIMD 并行计算
        for (; j <= n - 8; j += 8) {
            __m256 a_vec = _mm256_loadu_ps(&A[i * n + j]); // 加载 A 的 8 个元素（未对齐）
            __m256 x_vec = _mm256_loadu_ps(&x[j]);         // 加载 x 的 8 个元素（未对齐）
            // FMA 指令：sum += a_vec * x_vec，融合乘加提高效率
            sum = _mm256_fmadd_ps(a_vec, x_vec, sum);
        }
        // 将 256 位向量 sum 的 8 个元素提取并加和
        float temp[8];
        _mm256_storeu_ps(temp, sum);  // 存储到临时数组
        float total = temp[0] + temp[1] + temp[2] + temp[3] +
                      temp[4] + temp[5] + temp[6] + temp[7];
        // 清理循环：处理剩余元素（n 不是 8 的倍数时）
        for (; j < n; ++j) {
            total += A[i * n + j] * x[j];
        }
        y[i] = alpha * total + beta * y[i];  // 更新结果
    }
}

// 汇编接口：声明外部汇编函数
// 注意：符号名去掉下划线，汇编文件中定义的是_gemv_kernel  适配你的 Apple Clang 环境
extern "C" void gemv_kernel(float* A, float* x, float* y, int m, int n, float alpha, float beta);

// 汇编实现包装函数
void gemv_asm(float alpha, const std::vector<float>& A, const std::vector<float>& x,
              float beta, std::vector<float>& y, int m, int n) {
    // 调用汇编实现的 GEMV 函数，传递原始指针
    gemv_kernel(const_cast<float*>(A.data()), const_cast<float*>(x.data()),
                y.data(), m, n, alpha, beta);
}

// 测试框架：比较不同实现的性能和正确性
void test_gemv(void (*gemv_func)(float, const std::vector<float>&, const std::vector<float>&,
                                 float, std::vector<float>&, int, int),
               const std::string& name) {
    int m = 1024, n = 1024;  // 测试矩阵维度：1024×1024
    std::vector<float> A(m * n), x(n), y(m);  // 输入矩阵和向量
    float alpha = 1.0f, beta = 0.0f;          // 测试用的标量值

    // 初始化随机数据
    std::random_device rd;                    // 随机种子
    std::mt19937 gen(rd());                   // 随机数生成器
    std::uniform_real_distribution<> dis(0.0, 1.0);  // 均匀分布 [0, 1)
    for (int i = 0; i < m * n; ++i) A[i] = dis(gen); // 填充矩阵 A
    for (int i = 0; i < n; ++i) x[i] = dis(gen);     // 填充向量 x
    for (int i = 0; i < m; ++i) y[i] = dis(gen);     // 填充向量 y

    // 计时开始
    auto start = std::chrono::high_resolution_clock::now();
    gemv_func(alpha, A, x, beta, y, m, n);  // 执行 GEMV
    auto end = std::chrono::high_resolution_clock::now();
    // 计算耗时（微秒）
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << name << " took " << duration.count() << " microseconds\n";

    // 输出结果前 5 个元素，验证正确性
    std::cout << "First few elements of y: ";
    for (int i = 0; i < std::min(5, m); ++i) std::cout << y[i] << " ";
    std::cout << "\n";
}

// 主函数：运行所有实现并比较
int main() {
    test_gemv(gemv_naive, "Naive");      // 测试朴素实现
    test_gemv(gemv_unrolled, "Unrolled"); // 测试循环展开
    test_gemv(gemv_avx, "AVX");          // 测试 AVX 实现
    test_gemv(gemv_asm, "Assembly");     // 测试汇编实现
    return 0;
}