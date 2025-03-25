#include <iostream>
#include <vector>

int main() {
    std::vector<int8_t> vec_a = {1, 2, 3};
    int8_t* data_ptr = vec_a.data();

    for (size_t i = 0; i < vec_a.size(); ++i) {
        std::cout << static_cast<int>(data_ptr[i]) << " ";
    }

    return 0;
}
