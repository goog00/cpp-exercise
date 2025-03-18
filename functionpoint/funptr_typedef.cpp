#include <iostream>

// 使用typedef定义的函数指针
typedef void (*Operation)(int,int);
// 使用 using 定义的函数指针
using Operation =  void (*)(int,int);

void add(int a, int b) {
    std::cout << "Sum: " << (a + b) << std::endl;
}

int main() {
    // 使用typedef定义的函数指针
    Operation op = add;
    op(3,5);
    return 0;
}