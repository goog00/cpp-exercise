#include "BlockSizeCalculator.h"
#include <iostream>
#include <chrono> // 高精度计时器

// 分块矩阵乘法核心函数
template <typename T>
void block_matmul(const T* A, const T* B, T* C,
                  int rows_A, int cols_A, int rows_B, int cols_B,
                  int block_M, int block_K, int block_N) {
    for (int i = 0; i < rows_A; i += block_M) {
        for (int j = 0; j < cols_B; j += block_N) {
            for (int k = 0; k < cols_A; k += block_K) {
                for (int ii = i; ii < i + block_M && ii < rows_A; ++ii) {
                    for (int jj = j; jj < j + block_N && jj < cols_B; ++jj) {
                        T sum = 0;
                        for (int kk = k; kk < k + block_K && kk < cols_A; ++kk) {
                            sum += A[ii * cols_A + kk] * B[kk * cols_B + jj];
                        }
                        C[ii * cols_B + jj] += sum;
                    }
                }
            }
        }
    }
}

// 测量性能
template <typename T>
double measure_performance(const T* A, const T* B, T* C,
                           int rows_A, int cols_A, int rows_B, int cols_B,
                           int block_M, int block_K, int block_N,
                           int iterations = 10) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        // 清零矩阵 C
        for (int i = 0; i < rows_A * cols_B; ++i) C[i] = 0;

        // 执行分块矩阵乘法
        block_matmul(A, B, C, rows_A, cols_A, rows_B, cols_B,
                     block_M, block_K, block_N);
    }
    auto end = std::chrono::high_resolution_clock::now();

    // 计算平均运行时间（单位：毫秒）
    std::chrono::duration<double, std::milli> duration = end - start;
    return duration.count() / iterations;
}

// 主函数
template <typename T>
void run_matrix_multiplication(int rows_A, int cols_A, int rows_B, int cols_B) {
    T* A = new T[rows_A * cols_A]{};
    T* B = new T[rows_B * cols_B]{};
    T* C = new T[rows_A * cols_B]{};

    // 初始化矩阵（示例）
    for (int i = 0; i < rows_A * cols_A; ++i) A[i] = static_cast<T>(1.0);
    for (int i = 0; i < rows_B * cols_B; ++i) B[i] = static_cast<T>(1.0);

    // 获取缓存信息
    CacheInfo<T> L3_cache = BlockSizeCalculator<T>::get_cache_info("L3 cache");
    CacheInfo<T> L2_cache = BlockSizeCalculator<T>::get_cache_info("L2 cache");
    CacheInfo<T> L1_cache = BlockSizeCalculator<T>::get_cache_info("L1d cache");

    // 动态计算分块尺寸
    int M_L3, K_L3, N_L3;
    BlockSizeCalculator<T>::compute_block_sizes(L3_cache, rows_A, cols_A, rows_B, cols_B, M_L3, K_L3, N_L3);

    int M_L2, K_L2, N_L2;
    BlockSizeCalculator<T>::compute_block_sizes(L2_cache, rows_A, cols_A, rows_B, cols_B, M_L2, K_L2, N_L2);

    int M_L1, K_L1, N_L1;
    BlockSizeCalculator<T>::compute_block_sizes(L1_cache, rows_A, cols_A, rows_B, cols_B, M_L1, K_L1, N_L1);

    // 测量性能
    double time_L3 = measure_performance(A, B, C, rows_A, cols_A, rows_B, cols_B, M_L3, K_L3, N_L3);
    double time_L2 = measure_performance(A, B, C, rows_A, cols_A, rows_B, cols_B, M_L2, K_L2, N_L2);
    double time_L1 = measure_performance(A, B, C, rows_A, cols_A, rows_B, cols_B, M_L1, K_L1, N_L1);

    // 输出结果
    std::cout << "L3 Block Size: M=" << M_L3 << ", K=" << K_L3 << ", N=" << N_L3
              << ", Time: " << time_L3 << " ms" << std::endl;
    std::cout << "L2 Block Size: M=" << M_L2 << ", K=" << K_L2 << ", N=" << N_L2
              << ", Time: " << time_L2 << " ms" << std::endl;
    std::cout << "L1 Block Size: M=" << M_L1 << ", K=" << K_L1 << ", N=" << N_L1
              << ", Time: " << time_L1 << " ms" << std::endl;

    delete[] A; delete[] B; delete[] C;
}

int main() {
    // const int rows_A = 512, cols_A = 256;
    // const int rows_B = 256, cols_B = 512;

    // const int rows_A = 256, cols_A = 128;
    // const int rows_B = 128, cols_B = 256;

    const int rows_A = 128, cols_A = 64;
    const int rows_B = 64, cols_B = 128;
    // 测试 double 类型
    std::cout << "Testing with double type:" << std::endl;
    run_matrix_multiplication<double>(rows_A, cols_A, rows_B, cols_B);

    // 测试 float 类型
    std::cout << "\nTesting with float type:" << std::endl;
    run_matrix_multiplication<float>(rows_A, cols_A, rows_B, cols_B);

    // 测试 int 类型
    std::cout << "\nTesting with int type:" << std::endl;
    run_matrix_multiplication<int>(rows_A, cols_A, rows_B, cols_B);

    return 0;
}