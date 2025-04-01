#include <cstddef>
#include <cstdlib>
#include <iostream>

int main() {
    size_t alignment = 32;
    size_t requested_size = 17;

    size_t adjusted_size = (requested_size + alignment - 1) & ~(alignment - 1);
    std::cout << "Adjusted size: " << adjusted_size << std::endl;
    void* ptr = std::aligned_alloc(alignment, adjusted_size);
    if (ptr == nullptr) {
        std::cerr << "Memory allocation failed" << std::endl;
        return 1;
    }
    std::cout << "Memory allocated at: " << ptr << std::endl;
    std::free(ptr);
    return 0;

}