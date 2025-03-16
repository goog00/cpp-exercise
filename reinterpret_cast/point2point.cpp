#include <iostream>

struct A {
    int x;
};

//指针类型之间的转换
int main() {
    A a;
    void* ptr = &a;

    A* a_ptr = reinterpret_cast<A*>(ptr);
    a_ptr->x = 10;

    std::cout << "a.x = " << a.x << std::endl;
    return 0;
}