#include <iostream>
#include <memory>

struct MyClassB;
struct MyClassA {
  std::shared_ptr<MyClassB> b;
  ~MyClassA() { std::cout << "MyclassA::~MyClassA()\n"; }
};

struct MyClassB {
    std::shared_ptr<MyclassA> a;
    ~MyClassB() { std::cout << "MyClassB::~MyClassB()\n"; }
};


void useClassAandClassB() {
    auto a = std::make_shared<MyClassA>();
    auto b = std::make_shared<MyClassB>();
    a->b = b;
    b->a = a;

}


int main() {
    useClassAandClassB();
    std::cout << "Finished using A and B";
}