#include <iostream>
#include <memory>
#include <array>
#include <vector>
#include <Eigen/Dense>

class TensorType {
public:
    using VectorXi64 = Eigen::Matrix<int64_t, -1, 1>;

    TensorType(const VectorXi64& shape) : shape_(shape) {}

    static std::shared_ptr<TensorType> Create(std::shared_ptr<TensorType> base, const VectorXi64& shape) {
        return std::make_shared<TensorType>(shape);
    }

    void PrintShape() const {
        std::cout << "Tensor Shape: [ ";
        for (int i = 0; i < shape_.size(); ++i) {
            std::cout << shape_(i) << " ";
        }
        std::cout << "]" << std::endl;
    }

    const VectorXi64& GetShape() const { return shape_; }

private:
    VectorXi64 shape_;
};

class Tensor {
public:
    using VectorXi64 = Eigen::Matrix<int64_t, -1, 1>;

    Tensor(std::shared_ptr<TensorType> type) : type_(type) {}

    template <typename... Dims>
    std::shared_ptr<Tensor> Tile(Dims... dims) {
        std::array<int64_t, sizeof...(Dims)> shape_array = {dims...};
        std::cout << "shape_array.size():" << shape_array.size() << "\n";
        VectorXi64 shape(shape_array.size());
        for (size_t i = 0; i < shape_array.size(); ++i) {
            shape(i) = shape_array[i];
        }
        return std::make_shared<Tensor>(TensorType::Create(this->type_, shape));
    }

    void PrintShape() const {
        if (type_) {
            type_->PrintShape();
        }
    }

private:
    std::shared_ptr<TensorType> type_;
};

int main() {
    // 创建初始 TensorType (假设原始 shape 是 8x8)
    Eigen::Matrix<int64_t, -1, 1> init_shape(2);
    init_shape << 8, 8;
    auto tensor_type = std::make_shared<TensorType>(init_shape);

    // 创建 Tensor
    auto tensor = std::make_shared<Tensor>(tensor_type);

    // 执行 Tile 操作
    auto tiled_tensor1 = tensor->Tile(4, 1);
    auto tiled_tensor2 = tiled_tensor1->Tile(2, 1);

    // 打印结果
    std::cout << "Original ";
    tensor->PrintShape();

    std::cout << "After Tile(4,1) ";
    tiled_tensor1->PrintShape();

    std::cout << "After Tile(2,1) ";
    tiled_tensor2->PrintShape();





    return 0;
}
