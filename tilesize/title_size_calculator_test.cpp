// #include "galois_test.hpp"
// #include "galois/optimization/title_size_calculator.hpp"

// // 单元测试
// TEST(GaloisTests,test_standard_case) {
//     CacheConfig cache(32 * 1024, 256 * 1024, 8 * 1024 * 1024);  // 标准缓存大小
//     TileSizeCalculator calculator(cache);
//     TileSize ts = calculator.compute(512, 512, 512);

//     ASSERT_EQ(ts.ti_inner, 4);
//     ASSERT_EQ(ts.tj_inner, 4);
//     ASSERT_EQ(ts.ti_mid, 2);
//     ASSERT_EQ(ts.tj_mid, 2);
//     ASSERT_EQ(ts.tk_mid, 64);
//     ASSERT_EQ(ts.ti_outer, 64);  // 512 / (4 * 2) = 64
//     ASSERT_EQ(ts.tj_outer, 64);  // 512 / (4 * 2) = 64
// }

// TEST(GaloisTests,test_small_l1_cache) {
//     CacheConfig cache(2 * 1024, 256 * 1024, 8 * 1024 * 1024);  // 小的 L1 缓存
//     TileSizeCalculator calculator(cache);
//     TileSize ts = calculator.compute(512, 512, 512);

//     // L1 只有 512 floats (2048 / 4)，初始 528 > 512，需调整
//     ASSERT_EQ(ts.ti_inner, 2);  // 调整后 2 * 64 + 64 * 2 + 2 * 2 = 260 < 512
//     ASSERT_EQ(ts.tj_inner, 2);
//     ASSERT_EQ(ts.ti_mid, 2);
//     ASSERT_EQ(ts.tj_mid, 2);
//     ASSERT_EQ(ts.tk_mid, 64);
//     ASSERT_EQ(ts.ti_outer, 128);  // 512 / (2 * 2) = 128
//     ASSERT_EQ(ts.tj_outer, 128);  // 512 / (2 * 2) = 128
// }

// TEST(GaloisTests,test_small_l2_cache) {
//     CacheConfig cache(32 * 1024, 16 * 1024, 8 * 1024 * 1024);  // 小的 L2 缓存
//     TileSizeCalculator calculator(cache);
//     TileSize ts = calculator.compute(512, 512, 512);

//     // L2 只有 4096 floats，初始 1088 > 4096，需调整
//     ASSERT_EQ(ts.ti_inner, 4);
//     ASSERT_EQ(ts.tj_inner, 4);
//     ASSERT_EQ(ts.ti_mid, 1);  // 调整后 4 * 1 * 64 + 64 * 4 * 1 + 4 * 1 * 4 * 1 = 528 < 4096
//     ASSERT_EQ(ts.tj_mid, 1);
//     ASSERT_EQ(ts.tk_mid, 64);
//     ASSERT_EQ(ts.ti_outer, 128);  // 512 / (4 * 1) = 128
//     ASSERT_EQ(ts.tj_outer, 128);  // 512 / (4 * 1) = 128
// }

// TEST(GaloisTests,test_small_matrix) {
//     CacheConfig cache(32 * 1024, 256 * 1024, 8 * 1024 * 1024);
//     TileSizeCalculator calculator(cache);
//     TileSize ts = calculator.compute(16, 16, 16);

//     ASSERT_EQ(ts.ti_inner, 4);
//     ASSERT_EQ(ts.tj_inner, 4);
//     ASSERT_EQ(ts.ti_mid, 2);
//     ASSERT_EQ(ts.tj_mid, 2);
//     ASSERT_EQ(ts.tk_mid, 64);  // 不调整，因为 K 小仍使用初始值
//     ASSERT_EQ(ts.ti_outer, 2);  // 16 / (4 * 2) = 2
//     ASSERT_EQ(ts.tj_outer, 2);  // 16 / (4 * 2) = 2
// }


// TEST(GaloisTests,test_large_matrix_exceeds_l3) {
//     CacheConfig cache(32 * 1024, 256 * 1024, 1 * 1024 * 1024);  // 小的 L3 缓存
//     TileSizeCalculator calculator(cache);
//     TileSize ts = calculator.compute(1024, 1024, 1024);

//     // L3 = 262144 floats，矩阵 3 * 1024 * 1024 = 3145728 > 262144
//     ASSERT_EQ(ts.ti_inner, 4);
//     ASSERT_EQ(ts.tj_inner, 4);
//     ASSERT_EQ(ts.ti_mid, 2);
//     ASSERT_EQ(ts.tj_mid, 2);
//     ASSERT_EQ(ts.tk_mid, 64);
//     // L3 调整：262144 / (1024 + 1024 + 1) ≈ 127
//     ASSERT_EQ(ts.ti_outer, std::min(128, 127));  // 1024 / (4 * 2) = 128，但受 L3 限制
//     ASSERT_EQ(ts.tj_outer, std::min(128, 127));
// }