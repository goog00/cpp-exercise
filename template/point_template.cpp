#include <iostream>

// 定义一个带有非类型模板参数的类
template <typename T, int size>
class FixedArray {
    private:
        T array[size];
    public:
        T& operator[](int index) {
            return array[index];
        }
        int getSize() const {
            return size;
        }
};

int main(){
    FixedArray<int, 5> intArray;
    for (int i = 0; i < intArray.getSize(); ++i) {
        intArray[i] = i * 10;
    }

    for (int i = 0; i < intArray.getSize(); ++i) {
        std::cout << "intArray[" << i << "] = " << intArray[i] << std::endl;
    }
}