#include <cstdint>
#include <iostream>

int main() {
    int a = 42;
    int* p = &a;

    uintptr_t int_repr = reinterpret_cast<uintptr_t>(p);
    std::cout << "point as integer:" << int_repr << std::endl;


    //再转回来
    int* p2 = reinterpret_cast<int*>(int_repr);
    std::cout << "p2 point to :" << *p2 << std::endl;

    return 0;


}