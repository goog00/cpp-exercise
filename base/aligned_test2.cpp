#include <cstddef>
#include <cstdlib>
#include <iostream>

//分配特定类型的对齐内存：
int main() {
    size_t alignment = 16;
    size_t count = 5;// 分配5个元素
    size_t size = count * sizeof(double); // 计算总大小
    size_t adjuested_size = (size + alignment - 1) & ~(alignment - 1);

    double* ptr = static_cast<double*>(std::aligned_alloc(alignment,adjuested_size));

    if(ptr == nullptr) {
        std::cerr << "Memory allocation failed" << std::endl;
        return 1;
    }

    for(size_t i = 0; i < count ; ++i) {
        ptr[i] = i + 1.0; // 初始化分配的内存
    }

    std::cout << "Values: " ;
    for (size_t i = 0; i < count; ++i) {
        std::cout << ptr[i] << "";
    }

    std::cout << std::endl;
    std::free(ptr); // 释放内存
    return 0;


}