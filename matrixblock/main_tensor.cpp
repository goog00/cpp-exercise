#include "TileTensor.h"
#include <iostream>

int main() {
    // 假设我们有一个四维张量 A，其形状为 (4, 2, 1024, 128)
    // std::vector<int> tensor_shape = {4, 2, 1024, 128,512};
    std::vector<int> tensor_shape = {512,512};


    // 获取缓存信息
    auto cache_info = BlockSizeCalculator<float>::get_cache_info("L1 cache");

    // 计算分块大小
    std::vector<std::pair<int, int>> block_sizes;
    BlockSizeCalculator<float>::compute_block_sizes(cache_info, tensor_shape, block_sizes);

    // 输出分块策略
    std::cout << "Block sizes: ";
    for (const auto& block : block_sizes) {
        std::cout << "(" << block.first << ", " << block.second << ") ";
    }
    std::cout << std::endl;

    return 0;
}