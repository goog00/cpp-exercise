#include <gtest/gtest.h>
#include <vector>

// Step 1: 定义模板测试类
template <typename T>
class VectorTest : public ::testing::Test {
protected:
    std::vector<T> vec; // 每个测试实例都会有一个对应的 vector

    void SetUp() override {
        vec.push_back(static_cast<T>(10)); // 初始化向量
        vec.push_back(static_cast<T>(20));
    }

    void TearDown() override {
        vec.clear(); // 清理向量
    }
};

TYPED_TEST_SUITE_P(VectorTest);

// Step 2: 使用 TYPED_TEST_P 定义测试逻辑
TYPED_TEST_P(VectorTest, InitialSizeCheck) {
    EXPECT_EQ(this->vec.size(), 2); // 检查初始大小
}

TYPED_TEST_P(VectorTest, PushBackAndAccess) {
    this->vec.push_back(static_cast<TypeParam>(42)); // 插入一个值
    EXPECT_EQ(this->vec.size(), 3);                  // 检查大小
    EXPECT_EQ(this->vec[2], static_cast<TypeParam>(42)); // 检查值
}




// Step 3: 注册参数化测试套件
REGISTER_TYPED_TEST_SUITE_P(VectorTest, InitialSizeCheck, PushBackAndAccess);

// Step 4: 定义类型集合
using MyTypes = ::testing::Types<int, float, double>;

// Step 5: 实例化参数化测试套件
INSTANTIATE_TYPED_TEST_SUITE_P(MyVectorTests, VectorTest, MyTypes);