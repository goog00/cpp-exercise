// 3. 函数指针与数组
// 函数指针可以存储在数组中，用于动态调用不同的函数。
#include <iostream>

void greet() {
    std::cout << "Hello!" << std::endl;
}

void farewell() {
    std::cout << "Goodbye!" << std::endl;
}

int main() {
     // 函数指针数组

    void (*funcArr[2])() = {greet, farewell};

    funcArr[0]();  // 调用 greet
    funcArr[1]();  // 调用 farewell

    return 0;
}
