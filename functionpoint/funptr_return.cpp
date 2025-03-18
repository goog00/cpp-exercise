//返回函数指针
//函数也可以返回一个函数指针。

#include <iostream>
void add(int a, int b) {
    std::cout << "Sum: " << (a + b) << std::endl;
}

void multiply(int a, int b) {
    std::cout << "Product:" << (a * b) << std::endl;
}

// 返回函数指针
void (*getOperation(bool isAdd)) (int, int) {
    return isAdd ? add : multiply;
}

int main() {
    auto operation = getOperation(true);
    //调用add 函数
    operation(3,7);
    
    // 调用multiply
    operation = getOperation(false);
    operation(3,7);

    return 0;
}

