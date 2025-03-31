#include <gtest/gtest.h>
#include <vector>
#include <typeinfo>
#include <string>

// 定义参数结构体
template <typename T>
struct ParamWithType {
    T value;
};

// 打印参数值和类型信息（可选）
template <typename T>
std::ostream& operator<<(std::ostream& os, const ParamWithType<T>& param) {
    os << "{value=" << param.value << ", type=" << typeid(T).name() << "}";
    return os;
}

// 针对 int 类型的测试类
class IntVectorTest : public ::testing::TestWithParam<ParamWithType<int>> {
protected:
    std::vector<int> vec;
    void SetUp() override {
        vec.push_back(10);
        vec.push_back(20);
    }
};

TEST_P(IntVectorTest, PushAndCheck) {
    auto param = GetParam();
    vec.push_back(param.value);
    EXPECT_EQ(vec.back(), param.value);
}

// 实例化
INSTANTIATE_TEST_SUITE_P(
    IntValues,
    IntVectorTest,
    ::testing::Values(
        ParamWithType<int>{42},
        ParamWithType<int>{99}
    )
);


// 针对 float 类型的测试类
class FloatVectorTest : public ::testing::TestWithParam<ParamWithType<float>> {
protected:
    std::vector<float> vec;
    void SetUp() override {
        vec.push_back(1.0f);
        vec.push_back(2.0f);
    }
};

TEST_P(FloatVectorTest, PushAndCheck) {
    auto param = GetParam();
    vec.push_back(param.value);
    EXPECT_FLOAT_EQ(vec.back(), param.value);
}

// 实例化
INSTANTIATE_TEST_SUITE_P(
    FloatValues,
    FloatVectorTest,
    ::testing::Values(
        ParamWithType<float>{3.14f},
        ParamWithType<float>{6.28f}
    )
);
