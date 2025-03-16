// https://app.yinxiang.com/fx/381f90a7-0eb3-43d2-95c9-13fe167fb481
// 4. 非类型模板参数通常用于需要处理固定大小数据的场景，
// 比如在数组、固定大小的矩阵、或算法中。

#include <iostream>

template <typename T, int size>
class Array {
    private:
      T arr[size];

    public:
      T& operator[](int index) {
        return arr[index];
      }  
};

int main(){
    // 创建大小为5 的int 数组
    Array<int, 5> myArray;
    myArray[0] = 10;

    std::cout << myArray[0] << std::endl;

    return 0;
}