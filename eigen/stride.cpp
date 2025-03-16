#include <iostream>
#include <Eigen/Dense>

int main() {
    Eigen::Matrix<int,2,3, Eigen::RowMajor> mat;

    mat << 1,2,3,
           4,5,6;

    std::cout << "Matrix:\n" << mat << "\n";
    std::cout << "stride (out,inner): (" << mat.outerStride() << "," << mat.innerStride()   << ")\n";
    return 0;     

}