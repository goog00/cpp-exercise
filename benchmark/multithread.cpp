#include <algorithm>
#include <benchmark/benchmark.h>
#include <thread>
#include <vector>


//1. 多线程测试
static void BM_MultiThread(benchmark::State& state) {
    for (auto _ : state) {
        std::this_thread::sleep_for(std::chrono::microseconds(100));// 模拟工作

    }
}

//注册测试，指定线程数
BENCHMARK(BM_MultiThread)->Threads(4); //使用4个线程
BENCHMARK(BM_MultiThread)->ThreadRange(1,8);//线程从1到8


//2.自定义参数测试
static void BM_StringCopyWithSize(benchmark::State& state) {
    //state.range(0)：获取注册时指定的参数值。
    std::string x(state.range(0),'a');//创建长度为range(0)的字符串
    for(auto _ : state) {
        std::string copy(x);//定义变量copy 并初始化
    }
}

// 指定参数范围和倍增因子
// 测试复制不同长度字符串的性能。
//RangeMultiplier(4)：参数每次乘以 4（而不是默认的 2）。
//Range(4, 64)：测试参数从 4 到 64（4、16、64）。
BENCHMARK(BM_StringCopyWithSize)->RangeMultiplier(4)->Range(4,64);



//3.使用夹具（Fixture）
// 用途：适合测试需要复杂初始状态的情况。
//当需要初始化和清理资源时，使用夹具。
//VectorFixture：继承 benchmark::Fixture，
//定义初始化 (SetUp) 和清理 (TearDown)。
class VectorFixture : public benchmark::Fixture {
    public:
        void SetUp(const benchmark::State& state) override {
            vec.resize(1000,42);// 初始化一个1000元素的向量
        }

        void TearDown(const benchmark::State& state) override {
            vec.clear();// 清理
        }

        std::vector<int> vec;
};

//BENCHMARK_F：注册夹具测试，第一个参数是夹具类名，第二个是测试名。
// 测试在预初始化的向量上追加元素。
BENCHMARK_F(VectorFixture,BM_PushBack)(benchmark::State& state) {
    for(auto _ : state) {
        vec.push_back(1);
    }
}


//4. 控制迭代次数
//手动指定迭代次数，而不是让 Benchmark 自动调整。
static void BM_FixedIterations(benchmark::State& state) {
    for(auto _ : state){
        //benchmark::DoNotOptimize：防止编译器优化掉代码。
        benchmark::DoNotOptimize(state.iterations());//防止优化
    }
}
// 指定固定 1000 次迭代：强制每次测试运行 1000 次迭代。
BENCHMARK(BM_FixedIterations)->Iterations(1000);


//5.测量内存分配
static void BM_StringAlloc(benchmark::State& state) {
    for(auto _ : state) {
        std::string s = "hello world";
        //benchmark::DoNotOptimize：防止编译器优化掉代码。
        benchmark::DoNotOptimize(s);
    }
    //SetBytesProcessed：手动设置每次迭代处理的字节数，用于计算吞吐量。
    state.SetBytesProcessed(state.iterations() * sizeof("hello world"));
}

// -----------------------------------------------------------------------------
// Benchmark                                   Time             CPU   Iterations
// -----------------------------------------------------------------------------
// BM_StringAlloc                           25.1 ns         25.0 ns     27124949 bytes_per_second=456.935Mi/s

//吞吐量:bytes_per_second=456.935Mi/s


BENCHMARK(BM_StringAlloc);


// 6. 复杂参数组合
// 使用多组参数测试

#include <benchmark/benchmark.h>

static void BM_ComplexArgs(benchmark::State& state) {
    int size = state.range(0);  // 第一组参数
    int factor = state.range(1);  // 第二组参数
    std::string s(size, 'x');
    for (auto _ : state) {
        for (int i = 0; i < factor; ++i) {
            std::string copy(s);
        }
    }
}

// 使用 Arguments 指定参数对
//benchmark::internal::Benchmark 类的成员函数Args。Args({size, factor})：指定多组参数组合。
// 链式调用方法，
BENCHMARK(BM_ComplexArgs)->Args({8, 1})->Args({16, 2})->Args({32, 4});


BENCHMARK_MAIN();


