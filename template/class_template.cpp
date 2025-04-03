#include <iostream>
#include <vector>

// 类模板允许你定义一个通用的类，该类可以处理不同类型的对象。
// 定义一个类模板 Stack
template <typename T>
class Stack {

    private:
        std::vector<T> elements;
    public:
        void push(const T& element) {
            elements.push_back(element);
        }
        
        void pop() {
            elements.pop_back();
        }

        T top() const {
            return elements.back();
        }
        bool isEmpty() const {
            return elements.empty();
        }
};

int main() {
    Stack<int> intStack;
    intStack.push(1);
    intStack.push(2);
    intStack.push(3);
    
    std::cout << "Top element: " << intStack.top() << std::endl; // Output: 3
    intStack.pop();
    std::cout << "Top element after pop: " << intStack.top() << std::endl; // Output: 2

    Stack<std::string> stringStack;
    stringStack.push("Hello");
    stringStack.push("World");
    
    std::cout << "Top element: " << stringStack.top() << std::endl; // Output: World
    stringStack.pop();
    std::cout << "Top element after pop: " << stringStack.top() << std::endl; // Output: Hello

    return 0;
}