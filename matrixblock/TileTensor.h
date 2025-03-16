#ifndef BLOCK_SIZE_CALCULATOR_H
#define BLOCK_SIZE_CALCULATOR_H

#include <string>
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <vector>

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
    static void compute_block_sizes(const CacheInfo<T>& cache,
                                    const std::vector<int>& tensor_shape,
                                    std::vector<std::pair<int, int>>& block_sizes);
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

// 根据缓存大小和张量维度动态计算分块尺寸
template <typename T>
void BlockSizeCalculator<T>::compute_block_sizes(const CacheInfo<T>& cache,
                                                 const std::vector<int>& tensor_shape,
                                                 std::vector<std::pair<int, int>>& block_sizes) {
    int num_dims = tensor_shape.size();
    block_sizes.resize(num_dims, {1, 1}); // 初始化分块大小为 (1, 1)

    int cache_size = cache.size;
    int usable_cache = static_cast<int>(cache.size * 0.8); // 使用80%缓存空间

    // 计算最大允许的块存储空间
    int max_block_size = usable_cache / sizeof(T);

    // 初始化分块参数（基于启发式方法）
    for (int dim = 0; dim < num_dims; ++dim) {
        int dim_size = tensor_shape[dim];
        block_sizes[dim].first = 1; // 初始分块大小为 1
        block_sizes[dim].second = 1;

        // 动态调整分块大小
        if (dim_size > 1) {
            int block_dim = static_cast<int>(std::pow(max_block_size, 1.0 / num_dims));
            block_dim = std::min(block_dim, dim_size); // 防止超过维度大小
            block_dim = std::max(1, block_dim);        // 至少为 1

            // 根据缓存层级调整
            if (cache.size > 1024 * 1024) { // L3缓存
                block_dim = std::min(block_dim * 2, dim_size);
            } else if (cache.size > 256 * 1024) { // L2缓存
                block_dim = std::min(block_dim, dim_size);
            } else { // L1缓存
                block_dim = std::max(1, block_dim / 2);
            }

            // 确保分块能整除维度大小
            block_dim = dim_size / static_cast<int>(std::ceil(static_cast<double>(dim_size) / block_dim));

            // 设置分块大小
            block_sizes[dim].first = block_dim;
            block_sizes[dim].second = 1; // 固定为 1
        }
    }

    // 最终验证约束条件
    int total_block_memory = 0;
    for (int dim = 0; dim < num_dims; ++dim) {
        total_block_memory += block_sizes[dim].first * block_sizes[dim].second;
    }

    while (total_block_memory * sizeof(T) > usable_cache) {
        for (int dim = 0; dim < num_dims; ++dim) {
            block_sizes[dim].first /= 2;
            block_sizes[dim].first = std::max(1, block_sizes[dim].first);
        }

        total_block_memory = 0;
        for (int dim = 0; dim < num_dims; ++dim) {
            total_block_memory += block_sizes[dim].first * block_sizes[dim].second;
        }
    }
}

#endif // BLOCK_SIZE_CALCULATOR_H