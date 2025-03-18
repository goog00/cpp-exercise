#include <iostream>
// 指向不同的函数
//函数指针可以指向具有相同签名的任何函数。

void add(int a, int b){
    std::cout << "Sum: " << (a + b) << std::endl;
}

void multiply(int a ,int b) {
    std::cout << "Product: " << (a * b) << std::endl;
}

int main() {
    void (*operation)(int,int);

    operation = &add;
    //调用add 函数
    operation(3,5);

    operation = &multiply;

    //调用multiply函数
    operation(3,5);

    return 0;
}