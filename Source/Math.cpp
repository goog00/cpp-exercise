#include <iostream>

void Log(const char* message);

int Multiply(int a, int b)
{
    Log("Mutiply");
    int result = a * b;
    return result;

}  

int main(){
    std::cout << Multiply(5,8) << std::endl;
}


// #define INTEGER int  //将INTEGER 替换为 int 

// INTEGER Multiply(int a, int b)
// {
//     INTEGER result = a * b;
//     return result;
// #include "../Head/EndBrace.h"  // 预处理就只是把头文件的内容复制过来

// #if 1  // 如果true 则执行

// int Multiply(int a, int b)
// {
//     int result = a * b;
//     return result;

// }   
// #endif


