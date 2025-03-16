#include "TitleSizeCalculator.h"
#include <iostream>

// 打印分块格式
void print_tile_format(const TileSize& ts, int M, int N, int K) {
    std::cout << "Matrix A split format: ir::f32(" << ts.ti_inner << ", 1)(" << ts.ti_mid << ", 1)(1, " 
              << ts.tk_mid << ")(" << ts.ti_outer << ", 1)\n";
    std::cout << "Matrix B split format: ir::f32(1, " << ts.tj_inner << ")(1, " << ts.tj_mid << ")(" 
              << ts.tk_mid << ", 1)(1, " << ts.tj_outer << ")\n";
    std::cout << "Verification:\n";
    std::cout << "  A: M = " << ts.ti_outer * ts.ti_mid * ts.ti_inner << ", K = " << ts.tk_mid * (K / ts.tk_mid) << "\n";
    std::cout << "  B: K = " << ts.tk_mid * (K / ts.tk_mid) << ", N = " << ts.tj_outer * ts.tj_mid * ts.tj_inner << "\n";
}

// 2.1 分块矩阵乘法实现
void gemm_blocked(const float* A, const float* B, float* C, int M, int N, int K, const TileSize& ts) {
    // 外层循环 (L3 级别)
    for (int i0 = 0; i0 < M; i0 += ts.ti_outer * ts.ti_mid * ts.ti_inner) {
        for (int j0 = 0; j0 < N; j0 += ts.tj_outer * ts.tj_mid * ts.tj_inner) {
            for (int k0 = 0; k0 < K; k0 += ts.tk_mid) {
                // 中层循环 (L2 级别)
                for (int im = i0; im < std::min(i0 + ts.ti_outer * ts.ti_mid * ts.ti_inner, M); 
                     im += ts.ti_mid * ts.ti_inner) {
                    for (int jm = j0; jm < std::min(j0 + ts.tj_outer * ts.tj_mid * ts.tj_inner, N); 
                         jm += ts.tj_mid * ts.tj_inner) {
                        // 内层循环 (L1 级别)
                        for (int i = im; i < std::min(im + ts.ti_mid * ts.ti_inner, M); 
                             i += ts.ti_inner) {
                            for (int j = jm; j < std::min(jm + ts.tj_mid * ts.tj_inner, N); 
                                 j += ts.tj_inner) {
                                for (int k = k0; k < std::min(k0 + ts.tk_mid, K); k++) {
                                    // 计算 4x4 子块
                                    for (int p = i; p < std::min(i + ts.ti_inner, M); p++) {
                                        for (int q = j; q < std::min(j + ts.tj_inner, N); q++) {
                                            C[p * N + q] += A[p * K + k] * B[k * N + q];
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// 2.2 原生未分块矩阵乘法实现
void gemm_naive(const float* A, const float* B, float* C, int M, int N, int K) {
    // 简单三重循环，未优化
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < K; k++) {
                C[i * N + j] += A[i * K + k] * B[k * N + j];
            }
        }
    }
}

// 初始化矩阵
void init_matrix(float* mat, int rows, int cols, float base) {
    for (int i = 0; i < rows * cols; i++) {
        mat[i] = base + (i % 10);  // 简单初始化，避免全相同值
    }
}

// 打印矩阵（部分）
void print_matrix(const float* mat, int rows, int cols, int max_rows = 5, int max_cols = 5) {
    for (int i = 0; i < std::min(rows, max_rows); i++) {
        for (int j = 0; j < std::min(cols, max_cols); j++) {
            std::cout << mat[i * cols + j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

// 3. 测试用例（对比分块和未分块）
int main() {
    // 矩阵维度
    const int M = 512;
    const int N = 512;
    const int K = 512;

    // 缓存配置（自动检测）
    CacheConfig cache;
    std::cout << "Detected cache sizes: L1 = " << cache.l1_size / 1024 << "KB, L2 = " 
              << cache.l2_size / 1024 << "KB, L3 = " << cache.l3_size / (1024 * 1024) << "MB\n";

    // 创建分块大小计算器实例
    TileSizeCalculator calculator(cache);
    TileSize ts = calculator.compute(M, N, K);
    std::cout << "Calculated tile sizes:\n";
    print_tile_format(ts, M, N, K);

    // 分配矩阵内存
    std::vector<float> A(M * K);
    std::vector<float> B(K * N);
    std::vector<float> C_naive(M * N, 0.0f);  // 未分块结果
    std::vector<float> C_blocked(M * N, 0.0f);  // 分块结果

    // 初始化矩阵
    init_matrix(A.data(), M, K, 1.0f);
    init_matrix(B.data(), K, N, 2.0f);

    // 测试未分块实现
    std::cout << "Running naive GEMM...\n";
    clock_t start_naive = clock();
    gemm_naive(A.data(), B.data(), C_naive.data(), M, N, K);
    clock_t end_naive = clock();
    double time_naive = static_cast<double>(end_naive - start_naive) / CLOCKS_PER_SEC;

    // 测试分块实现
    std::cout << "Running blocked GEMM...\n";
    clock_t start_blocked = clock();
    gemm_blocked(A.data(), B.data(), C_blocked.data(), M, N, K, ts);
    clock_t end_blocked = clock();
    double time_blocked = static_cast<double>(end_blocked - start_blocked) / CLOCKS_PER_SEC;

    // 输出结果
    std::cout << "Naive GEMM execution time: " << time_naive << " seconds\n";
    std::cout << "Blocked GEMM execution time: " << time_blocked << " seconds\n";
    std::cout << "Performance improvement: " << (time_naive / time_blocked) << "x\n";

    // 验证结果一致性（打印前 5x5）
    std::cout << "Naive GEMM Matrix C (first 5x5):\n";
    print_matrix(C_naive.data(), M, N);
    std::cout << "Blocked GEMM Matrix C (first 5x5):\n";
    print_matrix(C_blocked.data(), M, N);

    // 检查结果是否一致（简单误差检查）
    bool consistent = true;
    for (int i = 0; i < M * N && consistent; i++) {
        if (std::abs(C_naive[i] - C_blocked[i]) > 1e-5) {
            consistent = false;
            std::cerr << "Error: Results differ at index " << i << "\n";
        }
    }
    if (consistent) {
        std::cout << "Results are consistent between naive and blocked implementations.\n";
    }

    return 0;
}