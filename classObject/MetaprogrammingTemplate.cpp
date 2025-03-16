// https://app.yinxiang.com/fx/381f90a7-0eb3-43d2-95c9-13fe167fb481
// 6.模板元编程
// 模板不仅用于通用编程，还可以用于元编程，即在编译时执行某些逻辑。
// 以下是一个简单的模板递归计算阶乘的例子：
#include <iostream>

template <int N>
struct Factorial {
    static const int value = N * Factorial<N - 1>::value;
};

template <>
struct Factorial<0> {
    static const int value = 1;
};

int main() {
    // 输出 120
    std::cout << Factorial<5>::value << std::endl;
    return 0；
}