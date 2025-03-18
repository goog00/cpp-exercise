// std::function（C++11）替代函数指针
// C++11 提供 std::function 作为函数指针的现代替代品，可以存储普通函数、Lambda 表达式和 std::bind 绑定的函数
#include <iostream>
#include <functional>

int add(int a, int b) {
    return a + b;
}

int main() {
    std::function<int(int, int)> func = add;  // 用 std::function 存储函数
    std::cout << "Result: " << func(4, 6) << std::endl;

    // 也可以存储 Lambda
    func = [](int x, int y) { return x * y; };
    std::cout << "Lambda result: " << func(4, 6) << std::endl;

    return 0;
}
