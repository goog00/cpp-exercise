#include <iostream>

void hello() {
    std::cout << "Hello world" << std::endl;


}

int main(){
    //函数指针
    void (*funcPtr)() = hello;
    //通过函数指针调用函数
    funcPtr();
    return 0;
}