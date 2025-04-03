#include <iostream>

template <typename T>
void swapValues(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}

int main() {
    int x = 5, y = 10;
    std::cout << "Before swap: x = " << x << ", y = " << y << std::endl;
    swapValues(x,y);
    std::cout << "After swap: x = " << x << ", y = " << y << std::endl;

    double m = 5.5, n = 10.5;
    std::cout << "Before swap: m = " << m << ", n = " << n << std::endl;
    swapValues(m,n);
    std::cout << "After swap: m = " << m << ", n = " << n << std::endl;
    return 0;
}