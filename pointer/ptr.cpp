#include <iostream>
#include <memory>

int main() {

  // 指针同样也是数据类型；
  // 不过他是变量地址的数据类型；
  // 白话就是说指针变量的值是地址
  int a = 10;
  // &（取地址运算符）：获取变量的地址
  //*（解引用运算符）：获取指针指向的值。(指针是地址，地址指向的值表示这个地址保存的值)
  int *p = &a;
  std::cout << "a的值： " << a << std::endl;
  std::cout << "a的地址：" << &a << std::endl;
  std::cout << "p的值(a的地址)：" << p << std::endl;
  std::cout << "*p的值（解引用）：" << *p << std::endl;

  // 2.1 指针的算术运算
  // 指针可以进行加减运算，每次递增或递减会跳转一个数据类型的大小。
  int arr[] = {1, 2, 3, 4, 5};
  int *p1 = arr; // 指向数组第一个元素

  std::cout << "p1 指向的值：" << *p1 << std::endl;
  p1++; // 指向下一个元素
  std::cout << "p1++ 后的值：" << *p1 << std::endl;

  // 3. 指针与动态内存分配
  // C++ 允许使用 new 和 delete 进行动态内存管理。
  int *p2 = new int(10);
  std::cout << "动态分配的值：" << *p2 << std::endl;

  delete p2; // 释放内存

  // 3.1 动态数组
  int *arr1 = new int[5]; // 分配5个整数的数组
  for (int i = 0; i < 5; i++) {
    arr[i] = i * 2;
  }
  delete[] arr1; // 释放动态数组

  std::unique_ptr<int[]> arr2 = std::make_unique<int[]>(5);
  for (int i = 0; i < 5; i++) {
    arr2[i] = i * 2;
  }

 


}