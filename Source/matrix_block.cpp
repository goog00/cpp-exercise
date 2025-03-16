#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <chrono> // 高精度计时器

// macOS 特定头文件
#ifdef __APPLE__
#include <sys/sysctl.h> // macOS sysctl API
#endif

template <typename T>
struct CacheInfo {
    int size;          // 缓存大小（字节）
    int line_size;     // 缓存行大小
    int associativity; // 关联度
    int latency;       // 访问延迟（纳秒）
};

// 获取缓存信息（兼容 macOS 和 Linux）
template <typename T>
CacheInfo<T> get_cache_info(const std::string& cache_type) {
    CacheInfo<T> info = {0};

#ifdef __linux__ // Linux 系统
    std::ifstream file("/proc/cpuinfo");
    if (!file.is_open()) return info;

    std::string line;
    while (std::getline(file, line)) {
        if (line.find(cache_type) != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string value_str = line.substr(pos + 1);
                int size_kb;
                if (std::sscanf(value_str.c_str(), "%dKB", &size_kb) == 1) {
                    info.size = size_kb * 1024; // 转换为字节
                }
            }
        } else if (line.find("cache line size") != std::string::npos) {
            int line_size;
            std::sscanf(line.c_str(), "%*[^:]: %d", &line_size);
            info.line_size = line_size;
        }
    }

#elif defined(__APPLE__) // macOS 系统
    size_t len = sizeof(info.size);

    if (cache_type == "L1d cache") {
        int mib[2] = {CTL_HW, HW_L1DCACHESIZE};
        sysctl(mib, 2, &info.size, &len, nullptr, 0);
    } else if (cache_type == "L2 cache") {
        int mib[2] = {CTL_HW, HW_L2CACHESIZE};
        sysctl(mib, 2, &info.size, &len, nullptr, 0);
    } else if (cache_type == "L3 cache") {
        int mib[2] = {CTL_HW, HW_L3CACHESIZE};
        sysctl(mib, 2, &info.size, &len, nullptr, 0);
    }

    // 默认缓存行大小（假设64字节）
    info.line_size = 64;

#else
#error "Unsupported platform"
#endif

    return info;
}

// 根据缓存大小和矩阵维度动态计算分块尺寸
template <typename T>
void compute_block_sizes(const CacheInfo<T>& cache, int rows_A, int cols_A,
                         int rows_B, int cols_B,
                         int& M, int& K, int& N) {
    int cache_size = cache.size;
    int usable_cache = static_cast<int>(cache.size * 0.8); // 使用80%缓存空间

    // 计算最大允许的块存储空间
    int max_block_size = usable_cache / sizeof(T);

    // 初始化分块参数（基于立方根启发式）
    K = static_cast<int>(std::cbrt(max_block_size));
    M = static_cast<int>(std::sqrt(max_block_size / K));
    N = static_cast<int>(std::sqrt(max_block_size / K));

    // 根据缓存层级调整（L3允许更大，L1更小）
    if (cache.size > 1024 * 1024) { // L3缓存
        K = std::min(K, cols_A); // 防止超过矩阵维度
        M = std::min(M * 2, rows_A); // 允许更大的行分块
        N = std::min(N * 2, cols_B);
    } else if (cache.size > 256 * 1024) { // L2缓存
        K = std::min(K, cols_A);
        M = std::min(M, rows_A);
        N = std::min(N, cols_B);
    } else { // L1缓存
        K = std::max(16, std::min(K / 2, cols_A));
        M = std::max(8, std::min(M / 2, rows_A));
        N = std::max(8, std::min(N / 2, cols_B));
    }

    // 确保分块能整除矩阵维度（或取接近的值）
    M = rows_A / static_cast<int>(std::ceil(static_cast<double>(rows_A) / M));
    N = cols_B / static_cast<int>(std::ceil(static_cast<double>(cols_B) / N));
    K = cols_A / static_cast<int>(std::ceil(static_cast<double>(cols_A) / K));

    // 最终验证约束条件
    while ((M * K + K * N) * sizeof(T) > usable_cache) {
        M /= 2;
        N /= 2;
        K /= 2;
    }
}

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

// 朴素矩阵乘法（未使用分块优化）
template <typename T>
void naive_matmul(const T* A, const T* B, T* C,
                  int rows_A, int cols_A, int rows_B, int cols_B) {
    for (int i = 0; i < rows_A; ++i) {
        for (int j = 0; j < cols_B; ++j) {
            T sum = 0;
            for (int k = 0; k < cols_A; ++k) {
                sum += A[i * cols_A + k] * B[k * cols_B + j];
            }
            C[i * cols_B + j] = sum;
        }
    }
}

// 测量矩阵乘法性能
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

// 测量朴素矩阵乘法性能
template <typename T>
double measure_naive_performance(const T* A, const T* B, T* C,
                                 int rows_A, int cols_A, int rows_B, int cols_B,
                                 int iterations = 10) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        // 清零矩阵 C
        for (int i = 0; i < rows_A * cols_B; ++i) C[i] = 0;

        // 执行朴素矩阵乘法
        naive_matmul(A, B, C, rows_A, cols_A, rows_B, cols_B);
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

    // 获取缓存信息（兼容 macOS 和 Linux）
    CacheInfo<T> L3_cache = get_cache_info<T>("L3 cache");
    CacheInfo<T> L2_cache = get_cache_info<T>("L2 cache");
    CacheInfo<T> L1_cache = get_cache_info<T>("L1d cache");

    // 动态计算分块尺寸
    int M_L3, K_L3, N_L3;
    compute_block_sizes(L3_cache, rows_A, cols_A, rows_B, cols_B, M_L3, K_L3, N_L3);

    int M_L2, K_L2, N_L2;
    compute_block_sizes(L2_cache, rows_A, cols_A, rows_B, cols_B, M_L2, K_L2, N_L2);

    int M_L1, K_L1, N_L1;
    compute_block_sizes(L1_cache, rows_A, cols_A, rows_B, cols_B, M_L1, K_L1, N_L1);

    // 测量性能
    double time_L3 = measure_performance(A, B, C, rows_A, cols_A, rows_B, cols_B, M_L3, K_L3, N_L3);
    double time_L2 = measure_performance(A, B, C, rows_A, cols_A, rows_B, cols_B, M_L2, K_L2, N_L2);
    double time_L1 = measure_performance(A, B, C, rows_A, cols_A, rows_B, cols_B, M_L1, K_L1, N_L1);

    // 测量朴素矩阵乘法性能
    double time_naive = measure_naive_performance(A, B, C, rows_A, cols_A, rows_B, cols_B);

    // 输出结果
    std::cout << "L3 Block Size: M=" << M_L3 << ", K=" << K_L3 << ", N=" << N_L3
              << ", Time: " << time_L3 << " ms" << std::endl;
    std::cout << "L2 Block Size: M=" << M_L2 << ", K=" << K_L2 << ", N=" << N_L2
              << ", Time: " << time_L2 << " ms" << std::endl;
    std::cout << "L1 Block Size: M=" << M_L1 << ", K=" << K_L1 << ", N=" << N_L1
              << ", Time: " << time_L1 << " ms" << std::endl;
    std::cout << "Naive Matrix Multiplication: Time: " << time_naive << " ms" << std::endl;

    delete[] A; delete[] B; delete[] C;
}

int main() {
    const int rows_A = 1024, cols_A = 512;
    const int rows_B = 512, cols_B = 1024;

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