// template <typename type> ret-type func-name(parameter list)
// {
//    // 函数的主体
// }
// type 是函数所使用的数据类型的占位符。 
// ret-type 返回类型
// 

#include <iostream>
#include <string>
using namespace std;


// 这段代码定义了一个模板函数Max，用于比较并返回两个给定参数中的最大值。其中：
// template <typename T>：表示这是一个模板函数，可以用于处理不同类型的参数。
// inline：表示该函数是内联函数，可能会在调用点进行展开，以提高性能。
// T const&：表示函数的参数是常量引用，以避免不必要的拷贝操作。
// a < b? b : a：使用条件运算符来确定返回哪个参数作为最大值。
template <typename T> 
inline T const& Max(T const& a,T const& b) {
    return a < b ? b : a;
}

int main() {
    int i = 39;
    int j = 20;
    cout << "Max(i,j):" << Max(i,j) << endl;
    
    double f1 = 13.5;
    double f2 = 20;

    cout << "Max(f1,f2) : " << Max(f1,f2) << endl;

    string s1 = "Hello";
    string s2 = "World";

    cout << "Max(s1,s2):" << Max(s1,s2) << endl;

    return 0;

}