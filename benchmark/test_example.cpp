#include <gtest/gtest.h>

TEST(MyTest, Test1) {
    ASSERT_EQ(1 + 1, 2);
}

TEST(MyTest, Test2) {
    ASSERT_TRUE(true);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}