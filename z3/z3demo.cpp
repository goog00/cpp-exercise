#include <iostream>
#include "z3++.h"

using namespace std;
using namespace z3;

int main() {
    context c;

    // 定义两个整数变量 x 和 y
    expr x = c.int_const("x");
    expr y = c.int_const("y");

    // 创建求解器
    solver s(c);

    // 添加约束条件
    s.add(x > y);
    s.add(y > 0);
    s.add(x == 2 * y);

    // 检查是否可满足
    if (s.check() == sat) {
        model m = s.get_model();
        cout << "Solution: " << endl;
        cout << "x = " << m.eval(x) << endl;
        cout << "y = " << m.eval(y) << endl;
    } else {
        cout << "No solution." << endl;
    }

    return 0;
}