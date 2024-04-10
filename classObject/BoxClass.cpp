#include <iostream>

using namespace std;

class Box {

public:
  double length;
  double breadth;
  double height;
  // 成员函数声明
  double get(void);
  void set(double len, double bre, double hei);

  // 成员函数可以定义在类定义内部，或者单独使用范围解析运算符 :: 来定义
  // 在类定义中定义的成员函数把函数声明为内联的，即便没有使用 inline 标识符
  double getVolume(void) { return length * breadth * height; }

}; // 分号结束一个类

// 用范围解析运算符 :: 来定义成员函数
double Box::getVolume(void) { return length * breadth * height; }

// 成员函数定义
// 范围解析运算符 ::
double Box::get(void) { return length * breadth * height; }

void Box::set(double len, double bre, double hei) {
  length = len;
  breadth = bre;
  height = hei;
}

int main() {
  // 声明
  Box Box1;
  Box Box2;
  Box Box3;
  // 用于存储体积
  double volume = 0.0;

  // box 1 详述
  Box1.height = 5.0;
  Box1.length = 6.0;
  Box1.breadth = 7.0;

  // box 2 详述
  Box2.height = 10.0;
  Box2.length = 12.0;
  Box2.breadth = 13.0;

  // box 1 的体积
  volume = Box1.height * Box1.length * Box1.breadth;
  cout << "Box1 的体积：" << volume << endl;

  // box 2 的体积
  volume = Box2.height * Box2.length * Box2.breadth;
  cout << "Box2 的体积：" << volume << endl;

  // box 3 详述
  Box3.set(16.0, 8.0, 12.0);
  volume = Box3.get();
  cout << "Box3 的体积：" << volume << endl;

  return 0;
}