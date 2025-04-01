#include <cstdlib>
#include <iostream>
#include <cstddef>
#include <cstdlib>

void* auto_aligned_alloc(size_t len) {
    // 可用的对齐值，从高到底
    const size_t alignments[] = {32, 16, 8, 4};
    const size_t num_alignments = sizeof(alignments) / sizeof(alignments[0]);

    if (len < 4) {
        void* ptr = std::malloc(len);
        std::cout << "Allocated " << len << " bytes with no alignment (malloc)\n";
        return ptr;
    }

    for (size_t i = 0; i < num_alignments; ++i) {
        size_t alignment = alignments[i];
        // 检查长度是否是对齐值的倍数
        if (len % alignment == 0) {
            void* ptr = aligned_alloc(alignment, len);
            if (ptr) {
                std::cout << "Allocated " << len << " bytes with alignment " << alignment << "\n";
                return ptr;
            }
        }
    }

    size_t min_alignment = 4;  // 默认对齐值
    size_t adjusted_size = (len + min_alignment - 1) & ~(min_alignment - 1);
    if (adjusted_size < 8) adjusted_size = 8; // 强制最小 8 字节
    std::cout << "Adjusted size to " << adjusted_size << " bytes for alignment " << min_alignment
              << "\n";
    // 调整大小以满足最小对齐要求
    void* ptr = aligned_alloc(min_alignment, adjusted_size);
    if (ptr) {
        std::cout << "Allocated " << adjusted_size << " bytes with alignment " << min_alignment << "\n";
        return ptr;
    } else {
        std::cout << "Failed to allocate memory with alignment " << min_alignment << "\n";
    }

    return nullptr;

}  

#include <cstdlib>
#include <iostream>

int main() {
    void* ptr = aligned_alloc(1, 32);
    if (ptr) {
        std::cout << "Success: Allocated 8 bytes with alignment 4\n";
        std::free(ptr);
    } else {
        std::cout << "Failure: Could not allocate 8 bytes with alignment 4\n";
    }
    return 0;
}