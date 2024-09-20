#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    std::vector<int> alice{1, 2, 3};
    std::vector<int> bob{7, 8, 9 , 10};

    auto print = [](const int& n) {
            std::cout << ' ' << n;
    };

    //打印交换前的状态
    std::cout << "alice:";
    std::for_each(alice.begin(), alice.end(), print);

    std::cout << "\nBobby:";
    std::for_each(bob.begin(), bob.end(), print);
    std::cout << '\n';

    std::cout << "-- SWAP\n";
    std::swap(alice, bob);

    // 打印交换后的状态
    std::cout << "alice: ";
    std::for_each(alice.begin(), alice.end(), print);
    std::cout << "\nBobby:";
    std::for_each(bob.begin(), bob.end(), print);
    std::cout << "\n";


}