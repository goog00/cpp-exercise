#include <benchmark/benchmark.h>

static void BM_Add(benchmark::State& state) {
    int a = 1, b = 2;
    for (auto _ : state) {
        benchmark::DoNotOptimize(a + b);
    }
}

BENCHMARK(BM_Add)->Range(8, 8<<10);
BENCHMARK_MAIN();