#include <iostream>

//作为函数参数
//函数指针可以作为参数传递给其他函数，常用于回调机制。
void printSum(int a, int b) {
    std::cout << "Sum: " << (a + b) << std::endl;
}

void printProduct(int a, int b) {
    std::cout << "Product: " << (a * b) << std::endl;
}

//接收函数指针作为参数
void compute(int x, int y, void (*operation)(int ,int)) {
    //调用传入的函数指针
    operation(x,y);
}

int main() {
    // 传递 printSum
    compute(4, 5, printSum);
    compute(4,  5, printProduct);
    return 0;
}