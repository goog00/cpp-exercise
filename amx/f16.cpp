#include <cstddef>
#include <immintrin.h>
#include <cstdint>
#include <sys/syscall.h>
#include <unistd.h>
#include <iostream>
#include <cpuid.h>

int ROWS = 8;
int COLS = 8;
size_t STRIDE_FP16 = COLS * sizeof(__fp16);
size_t STRIDE_FP32 = COLS * sizeof(float);

size_t ARCH_REQ_XCOMP_PERM = 0x1023;
size_t XFEATURE_XTILEDATA = 18;

using float16_t = __fp16;

struct __tile_config {
    uint8_t palette_id;
    uint8_t start_row;
    uint8_t reserved[14];
    uint8_t rows[16];
    uint16_t colsb[16];
};

bool check_amx_support() {
    unsigned int eax, ebx, ecx, edx;
    __cpuid_count(7, 0, eax, ebx, ecx, edx);
    bool amx_tile = (edx & (1 << 24)) != 0;
    if (!amx_tile) {
        std::cerr << "CPU does not support AMX-TILE.\n";
        return false;
    }
    return true;
}

bool set_tiledata_use() {
    int result = syscall(SYS_arch_prctl, ARCH_REQ_XCOMP_PERM, XFEATURE_XTILEDATA);
    if (result != 0) {
        std::cerr << "Failed to enable AMX tile data, errno: " << errno << "\n";
        return false;
    }
    std::cout << "AMX tile data enabled.\n";
    return true;
}

void init_tile_config(__tile_config* tileinfo) {
    tileinfo->palette_id = 1;
    tileinfo->start_row = 0;
    for (int i = 0; i < 3; ++i) {
        tileinfo->rows[i] = ROWS;
        tileinfo->colsb[i] = COLS * 2;
    }
    _tile_loadconfig(tileinfo);
}

void matrix_multiply_8x8_fp16_amx(const float16_t* A, const float16_t* B, float* C) {
    static bool initialized = false;
    static alignas(64) __tile_config tile_data = {0};
    if (!initialized) {
        if (!check_amx_support() || !set_tiledata_use()) {
            exit(-1);
        }
        init_tile_config(&tile_data);
        initialized = true;
    }
    _tile_loadd(2, A, STRIDE_FP16);
    _tile_loadd(3, B, STRIDE_FP16);
    _tile_loadd(1, C, STRIDE_FP32);
    _tile_dpfp16ps(1, 2, 3);
    _tile_stored(1, C, STRIDE_FP32);
}

void print_matrix_fp16(const float16_t* mat) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            std::cout << static_cast<float>(mat[i * COLS + j]) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void print_matrix_fp32(const float* mat) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            std::cout << mat[i * COLS + j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

int main() {
    alignas(64) float16_t A[ROWS * COLS];
    alignas(64) float16_t B[ROWS * COLS];
    alignas(64) float C[ROWS * COLS];

    for (int i = 0; i < ROWS * COLS; ++i) {
        A[i] = static_cast<float16_t>(i);
        B[i] = static_cast<float16_t>(i);
        C[i] = 0.0f;
    }
    std::cout << "Matrix A:\n";
    print_matrix_fp16(A);
    std::cout << "Matrix B:\n";
    print_matrix_fp16(B);

    matrix_multiply_8x8_fp16_amx(A, B, C);

    std::cout << "Result Matrix C (FP32):\n";
    print_matrix_fp32(C);

    _tile_release();
    return 0;
}