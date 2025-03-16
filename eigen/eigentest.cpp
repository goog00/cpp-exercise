#include <iostream>
#include "Eigen/Dense"

int main() {
    Eigen::Matrix3f A;
    A << 1, 2, 3,
         4, 5, 6,
         7, 8, 9;
    
    Eigen::Vector3f b(1, 2, 3);

    // 计算 Ax = b 的解
    Eigen::Vector3f x = A.colPivHouseholderQr().solve(b);

    std::cout << "解 x:\n" << x << std::endl;
    return 0;
}
