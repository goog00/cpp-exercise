#include <iostream>
#include <memory>

//https://app.yinxiang.com/fx/5887f756-379f-4ce5-a2f1-7cce636d4c70
class MyClass {
    public:
      MyClass() {
        std::cout << "Myclass constructor" << std::endl;
      }
      ~MyClass() {
        std::cout << "MyClass Destructor" << std::endl;
      }

      void Display() {
        std::cout << "Hello, from MyClass" << std::endl;
      }

};

int main() {
    //创建对象并让 ptr1 管理
    std::shared_ptr<MyClass> ptr1 = std::make_shared<MyClass>();
    {
        // 共享ptr1 管理的对象
        std::shared_ptr<MyClass> ptr2 = ptr1;
        ptr2->Display();
        //输出引用计数
        std::cout << "ptr2 use count: " << ptr2.use_count() <<  std::endl;

    } // ptr2 离开作用域，引用计数减1

    // 输出引用计数
    std::cout << "ptr1 use count: " << ptr1.use_count() << std::endl;
    return 0;
}