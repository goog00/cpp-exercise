#include <iostream>

using namespace std;

int main(){
    //实际变量的声明
    int var = 20;
    //指针变量的声明
    int *ip;
    //在指针变量中存储var的地址
    ip = &var;

    cout << "value of var variable: ";
    cout << var << endl;

    //输出在指针变量中存储的地址
    cout << "Address stored in ip variable: ";
    cout << ip << endl;

    //访问指针中地址的值
    cout << "value of *ip variable: ";
    cout << *ip << endl;

    return 0;

}