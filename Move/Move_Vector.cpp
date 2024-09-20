#include <iostream>
#include <vector>
#include <string>




int main() {

    


    //在容器中使用 std::move
    //std::move 常用于将对象移动到容器中，以避免不必要的拷贝。
    //例如，在向 std::vector 添加元素时使用 std::move 可以提高效率：
    // 在这个例子中，std::move(str) 将 str 转换为右值引用，因此 str 的内容被移动到 vector 中，而 str 变为空。
    std::vector<std::string> v;
    std::string str = "hello, world";

    v.push_back(std::move(str));
    // v.push_back(str);

    std::cout << "str: " << str << std::endl;
    std::cout << "v[0]" << v[0] << std::endl;

    return 0;

}