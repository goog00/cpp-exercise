#include <iostream>
#include <vector>
#include <immintrin.h>
#include <chrono>
#include <random>
#include <cstdlib>

extern "C" void gemv_kernel(float* A, float* x, float* y, int m, int n, float alpha, float beta);

void gemv_asm(float alpha, const std::vector<float>& A, const std::vector<float>& x,
              float beta, std::vector<float>& y, int m, int n) {
    gemv_kernel(const_cast<float*>(A.data()), const_cast<float*>(x.data()),
                 y.data(), m, n, alpha, beta);
}

void test_gemv(void (*gemv_func)(float, const std::vector<float>&, const std::vector<float>&,
                                 float, std::vector<float>&, int, int),
               const std::string& name) {
    int m = 1024, n = 1024;
    float* A_raw = static_cast<float*>(aligned_alloc(32, m * n * sizeof(float)));
    if (!A_raw) { std::cerr << "Failed to allocate A_raw\n"; return; }
    float* x_raw = static_cast<float*>(aligned_alloc(32, n * sizeof(float)));
    if (!x_raw) { std::cerr << "Failed to allocate x_raw\n"; free(A_raw); return; }
    float* y_raw = static_cast<float*>(aligned_alloc(32, m * sizeof(float)));
    if (!y_raw) { std::cerr << "Failed to allocate y_raw\n"; free(A_raw); free(x_raw); return; }
    std::vector<float> A(A_raw, A_raw + m * n);
    std::vector<float> x(x_raw, x_raw + n);
    std::vector<float> y(y_raw, y_raw + m);
    float alpha = 1.0f, beta = 0.0f;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    for (int i = 0; i < m * n; ++i) A[i] = dis(gen);
    for (int i = 0; i < n; ++i) x[i] = dis(gen);
    for (int i = 0; i < m; ++i) y[i] = dis(gen);

    auto start = std::chrono::high_resolution_clock::now();
    gemv_func(alpha, A, x, beta, y, m, n);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << name << " took " << duration.count() << " microseconds\n";

    std::cout << "First few elements of y: ";
    for (int i = 0; i < std::min(5, m); ++i) std::cout << y[i] << " ";
    std::cout << "\n";

    free(A_raw);
    free(x_raw);
    free(y_raw);
}

int main() {
    test_gemv(gemv_asm, "Optimized Assembly (16 elements)");
    return 0;
}