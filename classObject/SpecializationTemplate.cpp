// https://app.yinxiang.com/fx/381f90a7-0eb3-43d2-95c9-13fe167fb481
// 3. 模板的特化（Template Specialization）

#include <iostream>
template <typename T>
class Box {
    private:
       T value;

    public:
        Box(T val) : value(val) {}
        T getValue() {
            return value;
        } 

        void printValue() {
            std::cout << "this is a value : " << value << std::endl;
        }  
};

//全特化
// 为特定类型定制模板的实现。
template <>
class Box<char> { //特化 char 类型的 Box
    private:
        char value;

    public:
       Box(char val) : value(val) {}

       char getValue() {
         return value;
       } 

       void printValue() {
        std::cout << "this is a char: " << value << std::endl;
       }   
};



// 指针类型的偏特化
template <typename T>
class Box<T*> {
    private:
     T* value;

    public:
    Box(T* val) : value(val) {}
    T* getValue() {
        return value;
    } 

    void printValue() {
        std::cout << "Pointer points to value: " << *value << std::endl;
    }
};

int main() {
    // Box<char> charBox('d');
    // charBox.printValue();

    int num = 42;
    //创建指向 int 的Box
    Box<int*> intPointerBox(&num);

    // 获取指针
    int* retrivedValue = intPointerBox.getValue();
    // 打印指针指向的值
    std::cout << "Pointer value: " << *retrivedValue << std::endl;
    // 调用偏特化的成员函数， 打印指针指向的值
    intPointerBox.printValue();

    return  0;
    
}