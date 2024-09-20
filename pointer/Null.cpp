#include <cstddef>
#include <iostream>
#include <type_traits>
#include <typeinfo>

class S;

int main(){
    int* p = NULL;
    int* p2 = static_cast<std::nullptr_t>(NULL);
    // void(*f)(int) 是一个函数指针的声明。
    // f 是一个指针，它指向一个接受一个 int 类型参数并返回 void 的函数
    // = NULL 将这个函数指针 f 初始化为 NULL。
    // 这意味着 f 目前不指向任何实际的函数，它是一个空指针。
    void(*f)(int) = NULL;
    int S::*mp = NULL;
    void(S::*mfp)(int) = NULL;
    auto nullvar = NULL;

    std::cout << "nullvar 的类型是： " << typeid(nullvar).name() << '\n';

    if constexpr(std::is_same_v<decltype(NULL), std::nullptr_t)
        std::cout << "NULL 以 std::nullptr_t 类型实现\n";
    else 
        std::cout << "NULL 以整数类型实现\n";  

    [](...){}(p, p2, f, mp, mfp);      
       
}