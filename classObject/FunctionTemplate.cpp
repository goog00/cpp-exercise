// https://app.yinxiang.com/fx/381f90a7-0eb3-43d2-95c9-13fe167fb481

// 1. 函数模板（Function Template）

#include <iostream>

template <typename T> // 或者 template <class T>
T add(T a, T b) {
    return a + b;
}


int main() {
    int a = 1, b = 2;
    double x = 1.5 , y = 2.5;

    std::cout << add(a, b) << std::endl;
    std::cout << add(x, y) << std::endl;
}