#include <iostream>
#include <vector>

class MyVector {

public:
    std::vector<int> data;


    //移动赋值运算符
    MyVector& operator = (MyVector&& other) noexcept {
        data = std::move(other.data);
        std::cout << "move assignment" << std::endl;
        return *this;
    }   

    void display() const {
        for(int val : data) {
            std::cout << val << "";
        }

        std::cout << std::endl;
    } 
};

int main() {
    MyVector vec1;
    vec1.data = {1, 2, 3};

    MyVector vec2;
    vec2 = std::move(vec1);

    vec2.display(); 
    return 0;
}