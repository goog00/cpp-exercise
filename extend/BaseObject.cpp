#include <iostream>

// 基类（父类）
class BaseClass {
public:
    // 基类的构造函数
    BaseClass(int value) : baseValue(value) {
        std::cout << "基类构造函数被调用，baseValue = " << baseValue << std::endl;
    }

    void BaseMethod() {
        std::cout << "基类方法被调用" << std::endl;
    }

private:
    int baseValue;
};

// 派生类
class DerivedClass : public BaseClass {
public:
    // 派生类的构造函数，可以调用基类的构造函数
    DerivedClass(int value, int derivedValue) : BaseClass(value), derivedValue(derivedValue) {
        std::cout << "派生类构造函数被调用，derivedValue = " << derivedValue << std::endl;
    }

    void DerivedMethod() {
        std::cout << "派生类方法被调用" << std::endl;
    }

private:
    int derivedValue;
};

int main() {
    // 创建派生类对象，会首先调用基类构造函数，然后调用派生类构造函数
    DerivedClass obj(42, 100);

    // 可以调用基类和派生类的方法
    obj.BaseMethod();
    obj.DerivedMethod();
    
    return 0;
}
