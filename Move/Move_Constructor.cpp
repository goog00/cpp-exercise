#include <iostream>
#include <string>

class MyClass {
public:
    std::string name;

    MyClass(std::string n) : name(std::move(n)) {
        std::cout << "constructor" << std::endl;
    } 

    //移动构造函数
    MyClass(MyClass&& other) noexcept : name(std::move(other.name)) {
        std::cout << "Move constructor" << std::endl;
    } 

    //禁用拷贝构造函数
    MyClass(const MyClass&) = delete;

    void display() const {
        std::cout << "Name:" << name << std::endl;
    }
};

int main() {
    MyClass obj1("hello");
    MyClass obj2(std::move(obj1)); // 触发移动构造函数

    obj2.display(); // Name:hello
    return 0;
}