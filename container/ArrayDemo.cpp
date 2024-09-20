#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <string>

int main() {
    std::array<int, 3> a1{{1,2,3}} ; //CWG 1270 修订前的 C++11 中要求双花括号
    std::array<int, 3> a2 = {1,2,3};  //= 后决不要求双花括号

    std::sort(a1.begin(), a1.end());
    std::ranges::reverse_copy(a2, std::ostream_iterator<int>(std::cout, " "));
    std::cout << '\n';

    std::array<std::string,2> a3{"E","\u018E"};
    for (const auto& s : a3)
      std::cout << s << ' ';
    std::cout << '\n';
    
      

}