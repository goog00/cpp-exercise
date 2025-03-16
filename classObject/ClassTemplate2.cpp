
#include <iostream>
// https://app.yinxiang.com/fx/381f90a7-0eb3-43d2-95c9-13fe167fb481
// 类模板的使用场景
// 类模板通常用于容器类（如 std::vector、std::list）和其他可以处理多种数据类型的类中。
// 它们使代码更加通用，减少重复

// 2. 类模板（Class Template）

//定义了一个类型参数 T
//Box<T>：类的定义中 T 是泛型类型，它可以是 int、double、std::string 等任意类型。
template <typename T>
class Box {
    private:
       T value;

    public:
        Box(T val) : value(val) {}
        T getValue() {
            return value;
        }   
};


int main() {
    //创建处理int类型的box
    Box<int> intBox(123);
    //创建处理 double 类型的box
    Box<double> doubleBox(123.45);

    std::cout << intBox.getValue() << std::endl;
    std::cout << doubleBox.getValue() << std::endl;
    
    return 0;
}