#include <iostream>         // 用于标准输入输出
#include <vector>           // 用于动态数组存储矩阵和向量
#include <immintrin.h>      // AVX2 指令集 intrinsics
#include <chrono>           // 用于性能计时
#include <random>           // 用于生成随机测试数据
#include <cstdlib>          // 用于 aligned_alloc 和 free

// 声明外部汇编函数
extern "C" void gemv_kernel(float* A, float* x, float* y, int m, int n, float alpha, float beta);

// GEMV 包装函数：调用汇编实现
void gemv_asm(float alpha, const std::vector<float>& A, const std::vector<float>& x,
              float beta, std::vector<float>& y, int m, int n) {
    // 假设 A, x, y 已对齐，直接传递原始指针给汇编函数
    gemv_kernel(const_cast<float*>(A.data()), const_cast<float*>(x.data()),
                y.data(), m, n, alpha, beta);
}

// 测试框架：评估汇编实现的性能和正确性
void test_gemv(void (*gemv_func)(float, const std::vector<float>&, const std::vector<float>&,
                                 float, std::vector<float>&, int, int),
               const std::string& name) {
    int m = 1024, n = 1024;  // 测试矩阵维度：1024×1024
    // 分配对齐内存（32 字节对齐，适配 AVX2 的 256 位寄存器）
    // 分配对齐内存并检查是否成功
    float* A_raw = static_cast<float*>(aligned_alloc(32, m * n * sizeof(float)));
    if (!A_raw) {
        std::cerr << "Failed to allocate A_raw" << std::endl;
        return;
    }
    float* x_raw = static_cast<float*>(aligned_alloc(32, n * sizeof(float)));
    if (!x_raw) {
        std::cerr << "Failed to allocate x_raw" << std::endl;
        free(A_raw);
        return;
    }
    float* y_raw = static_cast<float*>(aligned_alloc(32, m * sizeof(float)));
    if (!y_raw) {
        std::cerr << "Failed to allocate y_raw" << std::endl;
        free(A_raw);
        free(x_raw);
        return;
    }
    // float* A_raw = static_cast<float*>(aligned_alloc(32, m * n * sizeof(float)));
    // float* x_raw = static_cast<float*>(aligned_alloc(32, n * sizeof(float)));
    // float* y_raw = static_cast<float*>(aligned_alloc(32, m * sizeof(float)));
    std::vector<float> A(A_raw, A_raw + m * n);  // 用对齐内存构造向量 A
    std::vector<float> x(x_raw, x_raw + n);      // 用对齐内存构造向量 x
    std::vector<float> y(y_raw, y_raw + m);      // 用对齐内存构造向量 y
    float alpha = 1.0f, beta = 0.0f;             // 测试用的标量值

    // 初始化随机数据
    std::random_device rd;                       // 随机种子
    std::mt19937 gen(rd());                      // Mersenne Twister 随机数生成器
    std::uniform_real_distribution<> dis(0.0, 1.0);  // 均匀分布 [0, 1)
    for (int i = 0; i < m * n; ++i) A[i] = dis(gen); // 填充矩阵 A
    for (int i = 0; i < n; ++i) x[i] = dis(gen);     // 填充向量 x
    for (int i = 0; i < m; ++i) y[i] = dis(gen);     // 填充向量 y

    // 计时开始
    auto start = std::chrono::high_resolution_clock::now();
    gemv_func(alpha, A, x, beta, y, m, n);       // 执行 GEMV
    auto end = std::chrono::high_resolution_clock::now();
    // 计算耗时（微秒）
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << name << " took " << duration.count() << " microseconds\n";

    // 输出前 5 个元素，验证结果
    std::cout << "First few elements of y: ";
    for (int i = 0; i < std::min(5, m); ++i) std::cout << y[i] << " ";
    std::cout << "\n";

    // 释放对齐内存
    free(A_raw);
    free(x_raw);
    free(y_raw);
}

int main() {
    test_gemv(gemv_asm, "Optimized Assembly (32 elements, blocked)");  // 测试优化后的汇编实现
    return 0;
}