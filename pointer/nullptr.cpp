#include <cstddef>
#include <iostream>

void f(int*){
    std::cout << "整数指针重载\n";
}

void f(double*){
    std::cout << "double 指针重载\n";
}

void f(std::nullptr_t){ //nullptr_t 空指针字面量
    std::cout << "空指针重载\n";
}

int main() {
    int* pi{};
    double* pd{};

    f(pi);
    f(pd);
    f(nullptr);
    //f(0);//
    // f(NULL);



}
