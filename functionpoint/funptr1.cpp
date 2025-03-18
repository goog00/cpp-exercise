#include "iostream"

// 函数指针的定义 ： 
// 返回值类型 (*指针名称)(参数类型列表);

void greet() {
    std::cout << "hello world" << std::endl;
}

int main() {
    // 定义函数指针 void (*funPtr())() 指向greet
    // &greet获取函数地址
    void (*funPtr)() = &greet;
    //通过 函数指针 调用greet
    funPtr();
    return 0;
}

