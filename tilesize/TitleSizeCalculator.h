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


public:
    // 构造函数，接受缓存配置参数
    // cache_config 包含 L1、L2 和 L3 缓存的大小
    TileSizeCalculator(const CacheConfig& cache_config) : cache(cache_config) {}

    // 计算适合 L1、L2 和 L3 缓存的分块大小
    TileSize compute(int M, int N, int K) const {
        TileSize ts;
        const int float_size = sizeof(float);  // 每个 float 占 4 字节

        // ===================== L1 分块 (最内层) =====================
        // 目标: 保证计算密度高，数据尽可能保留在 L1 缓存中
        int l1_elements = cache.l1_size / float_size;  // L1 可存储的浮点数个数
        
        // 初始的 L1 分块大小选择:
        ts.ti_inner = 4;  // i 方向的内层分块
        ts.tj_inner = 4;  // j 方向的内层分块
        int tk_inner = 64; // k 方向（归约轴）的内层分块较大，以重用数据

        // 计算 L1 使用量: ti_inner * tk_inner (A 子块) +
        //                  tk_inner * tj_inner (B 子块) +
        //                  ti_inner * tj_inner (C 子块)
        int l1_usage = ts.ti_inner * tk_inner + tk_inner * ts.tj_inner + ts.ti_inner * ts.tj_inner;

        // 如果 L1 使用量超过 L1 缓存容量，则逐步减少 ti_inner 和 tj_inner
        while (l1_usage > l1_elements && ts.ti_inner > 1) {
            ts.ti_inner /= 2;
            ts.tj_inner /= 2;
            l1_usage = ts.ti_inner * tk_inner + tk_inner * ts.tj_inner + ts.ti_inner * ts.tj_inner;
        }

        // ===================== L2 分块 (中层) =====================
        // 目标: 控制 L1 分块的堆叠次数，确保数据可以在 L2 复用
        int l2_elements = cache.l2_size / float_size;  // L2 可存储的浮点数个数
        
        // 初始的 L2 分块大小选择:
        ts.ti_mid = 2;  // i 方向的中层分块
        ts.tj_mid = 2;  // j 方向的中层分块
        ts.tk_mid = 64; // k 方向的中层分块

        // 计算中层的总分块大小 (L1 分块 * L2 分块)
        int ti = ts.ti_inner * ts.ti_mid;
        int tj = ts.tj_inner * ts.tj_mid;

        // 计算 L2 使用量
        int l2_usage = ti * ts.tk_mid + ts.tk_mid * tj + ti * tj;

        // 如果 L2 使用量超过 L2 容量，则逐步减少 ti_mid 和 tj_mid
        while (l2_usage > l2_elements && ts.ti_mid > 1) {
            ts.ti_mid /= 2;
            ts.tj_mid /= 2;
            ti = ts.ti_inner * ts.ti_mid;
            tj = ts.tj_inner * ts.tj_mid;
            l2_usage = ti * ts.tk_mid + ts.tk_mid * tj + ti * tj;
        }

        // 如果 L2 仍然超出限制，减少 tk_mid 以减少 K 方向的数据量
        while (l2_usage > l2_elements && ts.tk_mid > 8) {
            ts.tk_mid /= 2;
            l2_usage = ti * ts.tk_mid + ts.tk_mid * tj + ti * tj;
        }

        // ===================== L3 分块 (外层) =====================
        // 目标: 覆盖整个矩阵，尽可能减少数据加载的开销
        int l3_elements = cache.l3_size / float_size;  // L3 可存储的浮点数个数

        // 计算外层 i 和 j 方向的分块大小 (确保整个矩阵覆盖)
        ts.ti_outer = M / (ts.ti_inner * ts.ti_mid);
        ts.tj_outer = N / (ts.tj_inner * ts.tj_mid);

        // 计算 L3 使用量: 整个矩阵 A, B, C
        int l3_usage = M * K + K * N + M * N;

        // 如果 L3 超出缓存容量，调整 ti_outer 和 tj_outer 以减少缓存需求
        if (l3_usage > l3_elements) {
            ts.ti_outer = std::min(ts.ti_outer, l3_elements / (K + N + 1));
            ts.tj_outer = std::min(ts.tj_outer, l3_elements / (M + K + 1));
        }

        return ts;
    }

private:
    const CacheConfig& cache; // 缓存配置 (包含 L1, L2, L3 缓存大小)
};

