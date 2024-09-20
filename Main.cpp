#include <iostream> //# 后面的是预处理语句
//声明Log函数：当需要调用另一个文件中的函数，首先需要声明这个函数，声明的意思就是说告诉编译器这个函数存在
//void Log(const char* message) 声明函数时，形参可以写也可以不写。
void Log(const char*);

int main(){
    // << 表示一个函数
    // std::cout.print("hello world").printf(std::endl);
    // std::cout << "hello world!" << std::endl;
    Log("hello world!");
    std::cin.get();
    return 0;
}