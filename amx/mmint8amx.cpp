#include <cstddef>
#include <immintrin.h>
#include <cstdint>
#include <sys/syscall.h>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <cstring>

int ROWS = 8;
int COLS = 8;
size_t STRIDE_INT8 = COLS * sizeof(int8_t);   // 8 字节
size_t STRIDE_INT32 = COLS * sizeof(int32_t); // 32 字节

struct __tile_config {
    uint8_t palette_id;
    uint8_t start_row;
    uint8_t reserved[14];
    uint8_t rows[16];
    uint16_t colsb[16];
};

bool set_tiledata_use() {
    if (syscall(SYS_arch_prctl, 0x1023, 18)) {
        std::cerr << "Failed to enable AMX tile data\n";
        return false;
    }
    std::cout << "AMX tile data enabled\n";
    return true;
}

void init_tile_config(__tile_config* tileinfo) {
    std::cout << "tileinfo addr: " << (void*)tileinfo << "\n";
    std::memset(tileinfo, 0, sizeof(__tile_config));
    tileinfo->palette_id = 1;
    tileinfo->start_row = 0;
    for (int i = 0; i < 3; ++i) {
        tileinfo->rows[i] = 8;
        tileinfo->colsb[i] = 8;
    }
    std::cout << "tileinfo rows: ";
    for (int i = 0; i < 16; ++i) {
        std::cout << (int)tileinfo->rows[i] << " ";
    }
    std::cout << "\ntileinfo colsb: ";
    for (int i = 0; i < 16; ++i) {
        std::cout << tileinfo->colsb[i] << " ";
    }
    std::cout << "\nBefore _tile_loadconfig\n";
    _tile_loadconfig(tileinfo);
    std::cout << "After _tile_loadconfig\n";
}

void matrix_multiply_8x8_int8_amx(const int8_t* A, const int8_t* B, int32_t* C) {
    std::unique_ptr<__tile_config> tile_data = std::make_unique<__tile_config>();
    if (!set_tiledata_use()) {
        exit(-1);
    }
    init_tile_config(tile_data.get());
    _tile_loadd(2, A, STRIDE_INT8);
    _tile_loadd(3, B, STRIDE_INT8);
    _tile_loadd(1, C, STRIDE_INT32);
    _tile_dpbssd(1, 2, 3);
    _tile_stored(1, C, STRIDE_INT32);
}

void print_matrix_int8(const int8_t* mat) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            std::cout << (int)mat[i * COLS + j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void print_matrix_int32(const int32_t* mat) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            std::cout << mat[i * COLS + j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

int main() {
    alignas(64) int8_t A[ROWS * COLS];
    alignas(64) int8_t B[ROWS * COLS];
    alignas(64) int32_t C[ROWS * COLS];

    for (int i = 0; i < ROWS * COLS; ++i) {
        A[i] = static_cast<int8_t>(i);
        B[i] = static_cast<int8_t>(i);
        C[i] = 0;
    }

    std::cout << "Matrix A:\n";
    print_matrix_int8(A);
    std::cout << "Matrix B:\n";
    print_matrix_int8(B);

    matrix_multiply_8x8_int8_amx(A, B, C);

    std::cout << "Result Matrix C (INT32):\n";
    print_matrix_int32(C);

    _tile_release();
    return 0;
}