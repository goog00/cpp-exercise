#include "iostream"

void hello() {
    std::cout << "hello, world!" << std::endl;

}

int main() {
    // 把 void* 转换为函数指针
    void* ptr = reinterpret_cast<void*>(&hello);
    void (*func_ptr)() = reinterpret_cast<void (*)()>(ptr);
    // 直接调用
    func_ptr();
}