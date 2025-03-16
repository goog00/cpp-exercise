#include <string.h>

#include <benchmark/benchmark.h>

std::string processOrder(const std::string& orderData) {
    //模拟订单处理逻辑
    std::string result  = "Processed:" + orderData;
    return result;
}

// 1. 基本单次调用测试
static void BM_ProcessOrder(benchmark::State& state) {
    std::string orderData = "Order12345";
    for(auto _ : state) {
        std::string result = processOrder(orderData);
        benchmark::DoNotOptimize(result); // 防止优化
    }

}

// 输出：报告每次调用的平均时间。
// ----------------------------------------------------------
// Benchmark                Time             CPU   Iterations
// ----------------------------------------------------------
// BM_ProcessOrder       15.4 ns         15.4 ns     43005468

BENCHMARK(BM_ProcessOrder);



//2.测试不同输入规模
// 如果接口性能与输入数据大小相关，可以用参数测试不同规模。
static void BM_ProcessOrderWithSize(benchmark::State& state) {
    int size = state.range(0);// 获取参数：订单数据长度
    std::string orderData(size,'A');//生成指定长度的输入
    for (auto _ : state) {
        std::string result = processOrder(orderData);
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(state.iterations()*size);//报告吞吐量

}

//// 测试 100 到 10000 字节
BENCHMARK(BM_ProcessOrderWithSize)->Range(100,10000);


//3. 多线程并发测试
//测试接口在多线程环境下的性能。

static void  BM_ProcessOrderMultiThread(benchmark::State& state) {
    std::string orderData = "Order12345";
    for(auto _ : state) {
        std::string result = processOrder(orderData);
        benchmark::DoNotOptimize(result);
    }
}

// Threads(4)：固定 4 个线程并发调用。
BENCHMARK(BM_ProcessOrderMultiThread)->Threads(4);//4线程

// ThreadRange(1, 16)：测试从 1 到 16 个线程的性能。
BENCHMARK(BM_ProcessOrderMultiThread)->ThreadRange(1,16); //1到16线程

//4.使用夹具（Fixture）测试复杂场景
// 如果接口需要初始化（例如数据库连接），可以用夹具。

class OrderProcessorFixture : public benchmark::Fixture {
    public:
        //SetUp：在测试前初始化数据（只运行一次）。
       void SetUp(const benchmark::State&) override {
        //模拟初始化。例如连接数据库
        orderData = "Order12345";
       }

       std::string orderData;

};

BENCHMARK_F(OrderProcessorFixture,BM_ProcessOrder)(benchmark::State& state) {
    for (auto _ : state) {
        std::string result = processOrder(orderData);
        benchmark::DoNotOptimize(result);
    }
}


BENCHMARK_MAIN();

