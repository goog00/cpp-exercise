#include <iostream>

int main() {
    std::cout << "Printable assii [32..126]:\n";
    for (char c{' '}; c <= '~'; ++c)
       std::cout << c << ((c + 1) % 32 ? ' ' : '\n');

    std::cout << '\n';   
}