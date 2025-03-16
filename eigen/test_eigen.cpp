#include <iostream>
#include <Eigen/Dense>  // Eigen 核心头文件

// 定义 VectorXi64 类型
using VectorXi64 = Eigen::Vector<int64_t, Eigen::Dynamic>;  // -1 等价于 Eigen::Dynamic

int main() {
    // 初始化 VectorXi64
    VectorXi64 ss(5);  // 创建一个大小为 5 的向量
    ss << 1, 2, 3, 4, 5;  // 使用逗号初始化器赋值

    // 测试 ss.array() 方法
    std::cout << "Original vector ss:\n" << ss << "\n\n";

    // 使用 array() 进行逐元素操作
    auto ss_array = ss.array();  // 获取 ArrayWrapper
    std::cout << "ss.array():\n" << ss_array << "\n\n";

    // 示例 1: 每个元素加 10
    auto plus_ten = ss_array + 10;
    std::cout << "ss.array() + 10:\n" << plus_ten << "\n\n";

    // 示例 2: 每个元素平方
    auto squared = ss_array.square();
    std::cout << "ss.array().square():\n" << squared << "\n\n";

    // 示例 3: 与另一个 array 相乘
    VectorXi64 ss2(5);
    ss2 << 2, 2, 2, 2, 2;
    auto multiplied = ss_array * ss2.array();
    std::cout << "ss.array() * ss2.array():\n" << multiplied << "\n";

    return 0;
}