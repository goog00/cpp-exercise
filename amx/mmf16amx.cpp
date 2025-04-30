#include <cstddef>
#include <immintrin.h>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <sys/syscall.h>
#include <unistd.h>

int ROWS = 8;
int COLS = 8;
size_t STRIDE = COLS * sizeof(__fp16); // FP16步幅 8*2 = 16 字节

using float16_t = __fp16;

struct __tile_config {
    uint8_t palette_id;
    uint8_t start_row;
    uint8_t reserved[14];
    uint16_t colsb[16];  //列字节数
    uint8_t rows[16];    //行数

};


bool set_tiledata_use() {
    if (syscall(SYS_arch_prctl, 0x1023, 18)) {
        std::cerr << "Failed to enable AMX tile data \n" ;
        return false;
    }

    return true
}

// 初始化 tile 配置 8x8
void init_tile_config(__tile_config* tileinfo) {
    tileinfo->palette_id = 1;
    tileinfo->start_row = 0;
    for (int i = 0; i < 3; ++i) {
        tileinfo->rows[i] = ROWS;
        tileinfo->colsb[i] = COLS * 2;
    }
    _tile_loadconfig(tileinfo)
}

// 初始化FP16矩阵
void init_matrix_fp16(float16_t* mat, float value) {
    for (int i = 0; i < ROWS * COLS; ++i) {
        mat[i] = static_cast<float16_t>(value);
    }
}

// 初始化 FP32 矩阵
void init_matrix_fp32(float* mat, float value) {
    for (int i = 0; i < ROWS * COLS; ++i) {
        mat[i] = value;
    }
}

// 打印 FP16 矩阵
void print_matrix_fp16(const float16_t* mat) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            std::cout << static_cast<float>(mat[i * COLS + j]) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

// 打印 FP32 矩阵
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

    // 定义矩阵
    alignas(64) float16_t A[ROWS * COLS];
    alignas(64) float16_t B[ROWS * COLS];
    alignas(64) float C[ROWS * COLS];

    // 启用 AMX
    if (!set_tiledata_use()) {
        return -1;
    }
    // 初始化 tile 配置
    __tile_config tile_data = {0};
    init_tile_config(&tile_data);
    // 初始化矩阵
    init_matrix_fp16(A, 2.0f);
    init_matrix_fp16(B, 3.0f);
    init_matrix_fp32(C, 0.0f);
    // 打印矩阵
    std::cout << "Matrix A:\n";
    print_matrix_fp16(A);
    std::cout << "Matrix B:\n";
    print_matrix_fp16(B);
    // 加载矩阵 A 和 B 到 tile
    _tile_loadd(2, A, STRIDE);
    _tile_loadd(3, B, STRIDE);
    _tile_loadd(1, C, COLS * sizeof(float));

    // 执行矩阵乘法 : 执行FP16点积，结果累加到f32
    _tile_dpfp16ps(1, 2, 3);

    // 将结果从 tile 中存储到 C 矩阵
    _tile_stored(1, C, COLS * sizeof(float));
    // 打印结果
    std::cout << "Result Matrix C:\n";
    print_matrix_fp32(C);
    // 释放 tile
    _tile_release();
    
    return 0;

}