#include "gtest/gtest.h"

// 计算器接口
class Calculator {
public:
    virtual ~Calculator() = default;
    virtual int Add(int a, int b) = 0;
    virtual int Multiply(int a, int b) = 0;
};

// 实现 1：基本计算器
class BasicCalculator : public Calculator {
public:
    int Add(int a, int b) override { return a + b; }
    int Multiply(int a, int b) override { return a * b; }
};

// 实现 2：偏移计算器
class OffsetCalculator : public Calculator {
public:
    int Add(int a, int b) override { return a + b + 1; }
    int Multiply(int a, int b) override { return (a * b) + 2; }
};

// 工厂函数
template <typename T>
Calculator* CreateCalculator() {
    return new T;
}

// 测试夹具模板
template <typename T>
class CalculatorTest : public testing::TestWithParam<std::tuple<int, int, int>> {
protected:
    CalculatorTest() : calc_(CreateCalculator<T>()) {}
    ~CalculatorTest() override { delete calc_; }
    Calculator* calc_;
};

// 测试参数
static std::tuple<int, int, int> test_params[] = {
    std::make_tuple(2, 3, 5),
    std::make_tuple(0, 5, 5),
    std::make_tuple(-1, 7, 6),
};

// 为每种类型定义测试
using BasicCalcTest = CalculatorTest<BasicCalculator>;
using OffsetCalcTest = CalculatorTest<OffsetCalculator>;

// 值参数化测试（替代 TYPED_TEST_P）
TEST_P(BasicCalcTest, AdditionWorks) {
    auto [a, b, expected] = GetParam();
    int result = calc_->Add(a, b);
    EXPECT_EQ(result, a + b);
}

TEST_P(BasicCalcTest, MultiplicationWorks) {
    auto [a, b, expected] = GetParam();
    int result = calc_->Multiply(a, b);
    EXPECT_EQ(result, a * b);
}

TEST_P(OffsetCalcTest, AdditionWorks) {
    auto [a, b, expected] = GetParam();
    int result = calc_->Add(a, b);
    EXPECT_EQ(result, a + b + 1);
}

TEST_P(OffsetCalcTest, MultiplicationWorks) {
    auto [a, b, expected] = GetParam();
    int result = calc_->Multiply(a, b);
    EXPECT_EQ(result, a * b + 2);
}

// 实例化参数化测试
INSTANTIATE_TEST_SUITE_P(BasicCalcParams, BasicCalcTest, testing::ValuesIn(test_params));
INSTANTIATE_TEST_SUITE_P(OffsetCalcParams, OffsetCalcTest, testing::ValuesIn(test_params));

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}