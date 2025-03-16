#include <benchmark/benchmark.h>

//这是一个基准测试函数，用于测量创建空字符串的性能。
// 目的：测量 std::string 默认构造函数的性能。
// benchmark::State& state 是 Google Benchmark 提供的一个对象，用于控制测试的执行和计时。
static void BM_StringCreation(benchmark::State& state) {
    // 循环：for(auto _ : state) 是一个特殊的循环语法，_ 是一个占位符（表示不使用循环变量）。
    // 这个循环由 Google Benchmark 控制，内部会多次迭代执行括号内的代码，以确保测量结果的统计稳定性。
     for(auto _ : state) {
        // 测试内容：每次迭代创建一个空字符串 std::string empty_string。由于它是局部变量，生命周期仅限于每次迭代，构造和析构的开销会被测量。
        std::string empty_string;
     }
}

// 功能：将 BM_StringCreation 函数注册为一个基准测试。
// 作用：BENCHMARK 是一个宏，告诉 Google Benchmark 在运行时执行这个测试。它会自动调用 BM_StringCreation，并记录每次迭代的时间。
// 输出：运行后，Google Benchmark 会报告这个函数的平均执行时间、CPU 时间和迭代次数。
BENCHMARK(BM_StringCreation);

// 功能：这是一个基准测试函数，用于测量从现有字符串复制构造新字符串的性能。
// 初始化：std::string x = "hello" 在循环外部定义，因此只初始化一次，不会计入每次迭代的计时。这是典型的基准测试模式，确保只测量循环内的代码。
// 循环：for(auto _ : state) 同上，由 Google Benchmark 控制迭代。
// 测试内容：每次迭代通过 std::string copy(x) 创建一个新字符串 copy，这是 std::string 的复制构造函数的调用。
// 目的：测量复制一个固定长度字符串（"hello"，5 个字符）的性能。
static void BM_StringCopy(benchmark::State& state) {
    std::string x = "hello";
    for(auto _ : state) {
        std::string copy(x);
    }
}

//注册并指定参数范围
// 功能：将 BM_StringCopy 注册为基准测试，并指定一个参数范围。
// 参数范围：Range(8, 8<<10) 表示测试会以不同的参数值运行多次。
// 8 是起始值。
// 8<<10 是结束值，<< 是左移运算符，8<<10 等于 8 * 2^10 = 8192。
// 默认情况下，参数会以 2 的倍数递增（8, 16, 32, ..., 8192）。
// 作用：state 对象会接收这些参数（通过 state.range(0) 获取），但在这个例子中，BM_StringCopy 并未使用这些参数。通常，参数会用来控制测试规模（例如字符串长度或迭代次数），但这里只是展示了语法，函数本身没有利用它。
// 输出：Google Benchmark 会为每个参数值运行一次测试，并报告结果。例如，它会输出
BENCHMARK(BM_StringCopy)->Range(8, 8<<10);

// 功能：这是 Google Benchmark 提供的标准主函数宏。
// 作用：它生成一个 main() 函数，负责初始化 Benchmark 框架，运行所有注册的基准测试，并输出结果。不需要手动编写 int main()。

BENCHMARK_MAIN();

// 示例输出
// ------------------------------------------------------
// Benchmark            Time             CPU   Iterations
// ------------------------------------------------------
// BM_StringCreation    10 ns           10 ns   71084124
// BM_StringCopy/8      52 ns           52 ns   10000000
// BM_StringCopy/16     53 ns           53 ns   10000000
// BM_StringCopy/32     54 ns           54 ns   10000000
// ...                  ...             ...     ...
// BM_StringCopy/8192   55 ns           55 ns   10000000

// 墙钟时间是测量一段代码从开始执行到结束所花费的总时间，包括所有可能的开销，例如：
// CPU 执行代码的时间。
// I/O 操作（如读写文件）的等待时间。
// 系统调度的延迟。
// 多线程或并发时的同步开销。
// 其他外部因素（如操作系统中断）。

// CPU（CPU 时间）：每次迭代中 CPU 实际执行代码的时间
