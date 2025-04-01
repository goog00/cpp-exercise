#include <cstdlib>
#include <iostream>

int main() {
    // 使用 aligned_alloc 分配内存
    size_t alignment = 16; // 对齐要求
    size_t size = 64;      // 分配大小
    void* ptr = std::aligned_alloc(alignment, size);

    if (ptr == nullptr) {
        std::cerr << "Memory allocation failed" << std::endl;
        return 1;
    }

    // 使用分配的内存
    std::cout << "Memory allocated at: " << ptr << std::endl;

    // 释放内存
    std::free(ptr);
    return 0;
}