#include <iostream>

class MyClass {
    public:
       void printMessage() {
         std::cout << "Hello from Myclass" << std::endl;
       }
};

int main() {
    MyClass obj;
    //成员函数指针
    void (MyClass::*funcPtr)() = &MyClass::printMessage;
    //通过对象调用成员函数
    (obj.*funcPtr)();
    return 0;
}