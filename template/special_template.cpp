#include <iostream>


// 通用模版
template <typename T>
T getMax(T a, T b) {
    return (a > b) ? a : b;
}

// 特化模版
template <>
const char* getMax(const char* a, const char* b) {
    return (strcmp(a, b) > 0) ? a : b;
}

int main() {
    int x = 5, y = 10;
    std::cout << "Max of " << x << " and " << y << " is: " << getMax(x, y) << std::endl;
    double m = 5.5, n = 10.5;
    std::cout << "Max of " << m << " and " << n << " is: " << getMax(m, n) << std::endl;
    const char* str1 = "Hello";
    const char* str2 = "World";
    std::cout << "Max of " << str1 << " and " << str2 << " is: " << getMax(str1, str2) << std::endl;
    return 0;
}
