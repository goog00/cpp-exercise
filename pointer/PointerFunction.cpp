#include <iostream>
#include <ctime>

using namespace std;

//声明函数 形参 是无符号的long 型指针
void getSeconds(unsigned long *par);

int main(){
    unsigned long sec;
    getSeconds(&sec);

    //输出实际的值
    cout << "Number of seconds :" << sec << endl;

    return 0;
}

void getSeconds(unsigned long *par){
    *par = time(NULL);
    return;
}