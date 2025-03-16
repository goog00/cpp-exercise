#ifndef BLOCK_SIZE_CALCULATOR_H
#define BLOCK_SIZE_CALCULATOR_H

#include <string>
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <cstring>

// macOS 特定头文件
#ifdef __APPLE__
#include <sys/sysctl.h> // macOS sysctl API
#endif

// 缓存信息结构体
template <typename T>
struct CacheInfo {
    int size;          // 缓存大小（字节）
    int line_size;     // 缓存行大小
    int associativity; // 关联度
    int latency;       // 访问延迟（纳秒）
};

// 分块大小计算器类
template <typename T>
class BlockSizeCalculator {
public:
    // 获取缓存信息（兼容 macOS 和 Linux）
    static CacheInfo<T> get_cache_info(const std::string& cache_type);

    // 根据缓存大小和矩阵维度动态计算分块尺寸
    static void compute_block_sizes(const CacheInfo<T>& cache, int rows_A, int cols_A,
                                    int rows_B, int cols_B,
                                    int& M, int& K, int& N);
};

// 获取缓存信息（兼容 macOS 和 Linux）
template <typename T>
CacheInfo<T> BlockSizeCalculator<T>::get_cache_info(const std::string& cache_type) {
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
void BlockSizeCalculator<T>::compute_block_sizes(const CacheInfo<T>& cache, int rows_A, int cols_A,
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

#endif // BLOCK_SIZE_CALCULATOR_H