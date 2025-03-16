#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <ctime>

// 跨平台缓存检测头文件
#ifdef __linux__
#include <unistd.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#endif

// 分块参数结构体
struct TileSize {
    int ti_inner;  // 内层 i 分块 (L1)
    int ti_mid;    // 中层 i 分块 (L2)
    int ti_outer;  // 外层 i 分块 (L3)
    int tj_inner;  // 内层 j 分块 (L1)
    int tj_mid;    // 中层 j 分块 (L2)
    int tj_outer;  // 外层 j 分块 (L3)
    int tk_mid;    // 中层 k 分块 (L2)
};

// 缓存参数结构体（单位：字节）
struct CacheConfig {
    int64_t l1_size;  // L1 缓存大小
    int64_t l2_size;  // L2 缓存大小
    int64_t l3_size;  // L3 缓存大小

    // 默认构造函数，使用典型值作为回退
    CacheConfig() : l1_size(32 * 1024), l2_size(256 * 1024), l3_size(8 * 1024 * 1024) {
        detect_cache_sizes();  // 尝试检测本地缓存大小
    }

    // 检测本地缓存大小
    void detect_cache_sizes() {
#ifdef __linux__
        // Linux 使用 sysconf 获取缓存大小
        l1_size = sysconf(_SC_LEVEL1_DCACHE_SIZE);
        l2_size = sysconf(_SC_LEVEL2_CACHE_SIZE);
        l3_size = sysconf(_SC_LEVEL3_CACHE_SIZE);
        if (l1_size <= 0 || l2_size <= 0 || l3_size <= 0) {
            std::cerr << "Warning: Failed to detect cache sizes on Linux, using defaults.\n";
            l1_size = 32 * 1024;
            l2_size = 256 * 1024;
            l3_size = 8 * 1024 * 1024;
        }
#elif defined(__APPLE__)
        // macOS 使用 sysctl 获取缓存大小
        size_t len = sizeof(int64_t);
        if (sysctlbyname("hw.l1dcachesize", &l1_size, &len, NULL, 0) != 0) {
            l1_size = 32 * 1024;  // 默认值
        }
        if (sysctlbyname("hw.l2cachesize", &l2_size, &len, NULL, 0) != 0) {
            l2_size = 256 * 1024;
        }
        if (sysctlbyname("hw.l3cachesize", &l3_size, &len, NULL, 0) != 0) {
            l3_size = 8 * 1024 * 1024;
        }
#else
        std::cerr << "Warning: Cache detection not supported on this platform, using defaults.\n";
#endif
    }
};

// 分块大小计算类
class TileSizeCalculator {
private:
    const CacheConfig& cache;  // 缓存配置引用

public:
    // 构造函数，接收缓存配置
    TileSizeCalculator(const CacheConfig& cache_config) : cache(cache_config) {}

    // 计算分块大小的接口
    TileSize compute(int M, int N, int K) const {
        TileSize ts;
        const int float_size = sizeof(float);  // 每个 float 占 4 字节

        // L1 分块：内层，初始值较大以保证计算量
        int l1_elements = cache.l1_size / float_size;
        ts.ti_inner = 4;  // 内层 i 分块初始值
        ts.tj_inner = 4;  // 内层 j 分块初始值
        int tk_inner = 64;  // 内层 k 分块初始值（归约轴较大以重用数据）
        int l1_usage = ts.ti_inner * tk_inner + tk_inner * ts.tj_inner + ts.ti_inner * ts.tj_inner;
        while (l1_usage > l1_elements && ts.ti_inner > 1) {
            ts.ti_inner /= 2;  // 若超出 L1，减小 ti 和 tj
            ts.tj_inner /= 2;
            l1_usage = ts.ti_inner * tk_inner + tk_inner * ts.tj_inner + ts.ti_inner * ts.tj_inner;
        }

        // L2 分块：中层，初始值较小控制内层块堆叠次数
        int l2_elements = cache.l2_size / float_size;
        ts.ti_mid = 2;  // 中层 i 分块初始值
        ts.tj_mid = 2;  // 中层 j 分块初始值
        ts.tk_mid = 64;  // 中层 k 分块初始值
        int ti = ts.ti_inner * ts.ti_mid;  // 中层总 ti
        int tj = ts.tj_inner * ts.tj_mid;  // 中层总 tj
        int l2_usage = ti * ts.tk_mid + ts.tk_mid * tj + ti * tj;
        while (l2_usage > l2_elements && ts.ti_mid > 1) {
            ts.ti_mid /= 2;  // 若超出 L2，减小中层参数
            ts.tj_mid /= 2;
            ti = ts.ti_inner * ts.ti_mid;
            tj = ts.tj_inner * ts.tj_mid;
            l2_usage = ti * ts.tk_mid + ts.tk_mid * tj + ti * tj;
        }
        while (l2_usage > l2_elements && ts.tk_mid > 8) {
            ts.tk_mid /= 2;
            l2_usage = ti * ts.tk_mid + ts.tk_mid * tj + ti * tj;
        }

        // L3 分块：外层，覆盖整个矩阵
        int l3_elements = cache.l3_size / float_size;
        ts.ti_outer = M / (ts.ti_inner * ts.ti_mid);  // 外层 i 分块
        ts.tj_outer = N / (ts.tj_inner * ts.tj_mid);  // 外层 j 分块
        int l3_usage = M * K + K * N + M * N;
        if (l3_usage > l3_elements) {
            std::cerr << "Warning: Matrix size exceeds L3 cache capacity!\n";
            ts.ti_outer = std::min(ts.ti_outer, l3_elements / (K + N + 1));
            ts.tj_outer = std::min(ts.tj_outer, l3_elements / (M + K + 1));
        }

        return ts;
    }
};

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