#include <iostream>
#include <algorithm>
#include <cstdint>
#include <ctime>
#include <stdexcept>

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

    CacheConfig() : l1_size(32 * 1024), l2_size(256 * 1024), l3_size(8 * 1024 * 1024) {
        detect_cache_sizes();
    }

    void detect_cache_sizes() {
#ifdef __linux__
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
        size_t len = sizeof(int64_t);
        if (sysctlbyname("hw.l1dcachesize", &l1_size, &len, NULL, 0) != 0) {
            l1_size = 32 * 1024;
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

// 分块大小计算类（模板化）
template <typename T>
class TileSizeCalculator {
private:
    const CacheConfig& cache;

    // 根据数据类型动态调整初始值
    int get_initial_inner_size() const {
        int base_size = 8;  // 基础值
        int type_size = sizeof(T);
        return std::max(1, base_size / type_size);  // 确保至少为 1
    }

    int get_initial_mid_size() const {
        return 2;  // 中层初始值保持较小
    }

    int get_initial_tk_mid() const {
        int base_size = 128;  // tk_mid 初始值较大以重用数据
        int type_size = sizeof(T);
        return std::max(8, base_size / type_size);  // 确保至少为 8
    }

public:
    TileSizeCalculator(const CacheConfig& cache_config) : cache(cache_config) {}

    TileSize compute(int M, int N, int K) const {
        // 边界检查
        if (M <= 0 || N <= 0 || K <= 0) {
            throw std::invalid_argument("Matrix dimensions M, N, K must be positive.");
        }

        TileSize ts;
        const int type_size = sizeof(T);

        // L1 分块：内层
        int l1_elements = cache.l1_size / type_size;
        ts.ti_inner = get_initial_inner_size();
        ts.tj_inner = get_initial_inner_size();
        int tk_inner = get_initial_tk_mid();
        int l1_usage = ts.ti_inner * tk_inner + tk_inner * ts.tj_inner + ts.ti_inner * ts.tj_inner;
        while (l1_usage > l1_elements && ts.ti_inner > 1) {
            ts.ti_inner /= 2;
            ts.tj_inner /= 2;
            l1_usage = ts.ti_inner * tk_inner + tk_inner * ts.tj_inner + ts.ti_inner * ts.tj_inner;
        }

        // L2 分块：中层
        int l2_elements = cache.l2_size / type_size;
        ts.ti_mid = get_initial_mid_size();
        ts.tj_mid = get_initial_mid_size();
        ts.tk_mid = get_initial_tk_mid();
        int ti = ts.ti_inner * ts.ti_mid;
        int tj = ts.tj_inner * ts.tj_mid;
        int l2_usage = ti * ts.tk_mid + ts.tk_mid * tj + ti * tj;
        while (l2_usage > l2_elements && ts.ti_mid > 1) {
            ts.ti_mid /= 2;
            ts.tj_mid /= 2;
            ti = ts.ti_inner * ts.ti_mid;
            tj = ts.tj_inner * ts.tj_mid;
            l2_usage = ti * ts.tk_mid + ts.tk_mid * tj + ti * tj;
        }
        while (l2_usage > l2_elements && ts.tk_mid > 8) {
            ts.tk_mid /= 2;
            l2_usage = ti * ts.tk_mid + ts.tk_mid * tj + ti * tj;
        }

        // L3 分块：外层
        int l3_elements = cache.l3_size / type_size;
        int inner_mid_i = ts.ti_inner * ts.ti_mid;
        int inner_mid_j = ts.tj_inner * ts.tj_mid;
        ts.ti_outer = (M + inner_mid_i - 1) / inner_mid_i;  // 向上取整避免遗漏
        ts.tj_outer = (N + inner_mid_j - 1) / inner_mid_j;
        int l3_usage = M * K + K * N + M * N;
        if (l3_usage > l3_elements) {
            std::cerr << "Warning: Matrix size exceeds L3 cache capacity for type size " << type_size << " bytes!\n";
            ts.ti_outer = std::min(ts.ti_outer, l3_elements / (K + N + 1));
            ts.tj_outer = std::min(ts.tj_outer, l3_elements / (M + K + 1));
        }

        return ts;
    }
};

// 简单的分块矩阵乘法性能测试
template <typename T>
double test_matrix_multiply(int M, int N, int K, const TileSize& ts) {
    std::vector<T> A(M * K, T(1.0));
    std::vector<T> B(K * N, T(1.0));
    std::vector<T> C(M * N, T(0.0));

    clock_t start = clock();
    for (int io = 0; io < ts.ti_outer; io++) {
        for (int jo = 0; jo < ts.tj_outer; jo++) {
            for (int im = 0; im < ts.ti_mid; im++) {
                for (int jm = 0; jm < ts.tj_mid; jm++) {
                    for (int i = 0; i < ts.ti_inner; i++) {
                        for (int j = 0; j < ts.tj_inner; j++) {
                            for (int k = 0; k < ts.tk_mid; k++) {
                                int row = io * ts.ti_mid * ts.ti_inner + im * ts.ti_inner + i;
                                int col = jo * ts.tj_mid * ts.tj_inner + jm * ts.tj_inner + j;
                                if (row < M && col < N && k < K) {
                                    C[row * N + col] += A[row * K + k] * B[k * N + col];
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    clock_t end = clock();
    return double(end - start) / CLOCKS_PER_SEC;
}

// 测试代码
int main() {
    CacheConfig cache;
    std::cout << "L1: " << cache.l1_size << " bytes, L2: " << cache.l2_size << " bytes, L3: " << cache.l3_size << " bytes\n";

    int M = 512, N = 192, K = 1024;

    // 测试 float 类型
    TileSizeCalculator<float> calc_float(cache);
    TileSize ts_float = calc_float.compute(M, N, K);
    std::cout << "\nTileSize for float:\n";
    std::cout << "ti_inner: " << ts_float.ti_inner << ", tj_inner: " << ts_float.tj_inner << "\n";
    std::cout << "ti_mid: " << ts_float.ti_mid << ", tj_mid: " << ts_float.tj_mid << ", tk_mid: " << ts_float.tk_mid << "\n";
    std::cout << "ti_outer: " << ts_float.ti_outer << ", tj_outer: " << ts_float.tj_outer << "\n";
    double time_float = test_matrix_multiply<float>(M, N, K, ts_float);
    std::cout << "Time for float: " << time_float << " seconds\n";

    // 测试 double 类型
    TileSizeCalculator<double> calc_double(cache);
    TileSize ts_double = calc_double.compute(M, N, K);
    std::cout << "\nTileSize for double:\n";
    std::cout << "ti_inner: " << ts_double.ti_inner << ", tj_inner: " << ts_double.tj_inner << "\n";
    std::cout << "ti_mid: " << ts_double.ti_mid << ", tj_mid: " << ts_double.tj_mid << ", tk_mid: " << ts_double.tk_mid << "\n";
    std::cout << "ti_outer: " << ts_double.ti_outer << ", tj_outer: " << ts_double.tj_outer << "\n";
    double time_double = test_matrix_multiply<double>(M, N, K, ts_double);
    std::cout << "Time for double: " << time_double << " seconds\n";

    // 测试 int 类型
    TileSizeCalculator<int> calc_int(cache);
    TileSize ts_int = calc_int.compute(M, N, K);
    std::cout << "\nTileSize for int:\n";
    std::cout << "ti_inner: " << ts_int.ti_inner << ", tj_inner: " << ts_int.tj_inner << "\n";
    std::cout << "ti_mid: " << ts_int.ti_mid << ", tj_mid: " << ts_int.tj_mid << ", tk_mid: " << ts_int.tk_mid << "\n";
    std::cout << "ti_outer: " << ts_int.ti_outer << ", tj_outer: " << ts_int.tj_outer << "\n";
    double time_int = test_matrix_multiply<int>(M, N, K, ts_int);
    std::cout << "Time for int: " << time_int << " seconds\n";

    return 0;
}