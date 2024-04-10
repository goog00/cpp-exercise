#include <iostream>

using namespace std;

int main() {
    //声明简单变量
    int i;
    double d;

    //声明引用变量
    //& 读作引用.   r是一个初始化为i的整型引用
    int &r = i;
    double &s =d;

    i = 5;

    cout << "value of i : " << i << endl;
    cout << "value of i reference :" << r << endl;
     d = 11.7;

     cout << "value of d : " << d << endl;
     cout << "value of d reference" << s << endl;

     return 0;


}