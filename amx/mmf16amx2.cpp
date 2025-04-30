#include <cstddef>
#include <immintrin.h>
#include <cstdint>
#include <sys/syscall.h>
#include <unistd.h>
#include <iostream>

#include <x86intrin.h>
#include <cstring>
#include <cpuid.h>


// 对齐的要求
// 定义矩阵大小
int ROWS = 8;
int COLS = 8;
size_t STRIDE_FP16 = COLS * sizeof(__fp16); // FP16步幅 8*2 = 16 字节
size_t STRIDE_FP32 = COLS * sizeof(float); // FP32步幅 8*4 = 32 字节



using float16_t = __fp16;


// TIlE配置结构体
// AMX（Advanced Matrix Extensions）在硬件上支持 8 个 tile 寄存器，编号从 tmm0 到 tmm7。
struct __tile_config {
    uint8_t palette_id;    // 配置模式
    uint8_t start_row;     // 起始行（用于分块计算）
    uint8_t reserved[14];  // 保留字节，用于对齐
    // 每个 tile 的行数:rows[i] 的确是第 i 个 tile 的行数，范围是 0 到 255（uint8_t 的取值范围）
    // AMX 当前支持 8 个 tile（tmm0 到 tmm7），所以只用前 8 个元素，后 8 个预留。
    // 如，rows[0] = 8 表示 tile 0 有 8 行。
    uint8_t rows[16];     
    uint16_t colsb[16];    // 每个 tile 的列字节数 
};

// Set_tiledata_use() - Invoke syscall to set ARCH_SET_STATE_USE 
bool set_tiledata_use() {
    if (syscall(SYS_arch_prctl, 0x1023, 18)) {
        std::cerr << "Failed to enable AMX tile data \n";
        return false;
    }
    return true;
}

// void init_tile_config (__tile_config* tileinfo) {
//     tileinfo->palette_id = 1;
//     tileinfo->start_row = 0;
//     for (int i = 0; i < 3; ++i) { // 配置 3 个 tile
//         tileinfo->rows[i] = ROWS;        // 8 行
//         tileinfo->colsb[i] = COLS * 2;   // 列字节数：8 列 * 2字节（FP16) 16 字节
//     }

//     std::cout << "tileinfo rows: ";
//     for (int i = 0; i < 16; ++i) {
//         std::cout << (int)tileinfo->rows[i] << " ";
//     }
//     std::cout << "\n";

//      _tile_loadconfig(tileinfo); // 加载 tile 配置
// }

void init_tile_config(__tile_config* tileinfo) {
    std::memset(tileinfo, 0, sizeof(__tile_config));  // 清零

    tileinfo->palette_id = 0;  // 先尝试 0，看是否默认支持
    for (int i = 0; i < 3; ++i) {
        tileinfo->rows[i] = 8;
        tileinfo->colsb[i] = 16;  // FP16 需要 16 字节
    }

    std::cout << "tileinfo rows: ";
    for (int i = 0; i < 16; ++i) {
        std::cout << tileinfo->rows[i] << " ";
    }
    std::cout << "\ntileinfo colsb: ";
    for (int i = 0; i < 16; ++i) {
        std::cout << tileinfo->colsb[i] << " ";
    }
    std::cout << std::endl;

    _tile_loadconfig(tileinfo);
}

// 
void matrix_multiply_8x8_fp16_amx(const float16_t* A, const float16_t* B, float* C) {
     // 初始化 tile 配置
     alignas(64) __tile_config tile_data = {0};
      
     // Request permission to linux kernel to run AMX
    if (!set_tiledata_use())
        exit(-1);

     init_tile_config(&tile_data);

     // 加载矩阵 A 和 B 到 tile
     _tile_loadd(2, A, STRIDE_FP16);
     _tile_loadd(3, B, STRIDE_FP16);
     _tile_loadd(1, C, STRIDE_FP32);

    // 执行矩阵乘法 : 执行FP16点积，结果累加到f32
    _tile_dpfp16ps(1, 2, 3);
    // 将结果从 tile 中存储到 C 矩阵
    _tile_stored(1, C, STRIDE_FP32);
}

// 示例用法
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
    // 打印矩阵 A 和 B
    std::cout << "Matrix A:\n";
    print_matrix_fp16(A);
    std::cout << "Matrix B:\n";
    print_matrix_fp16(B);

    matrix_multiply_8x8_fp16_amx(A, B, C);

    // 打印结果
    std::cout << "Result Matrix C (FP32):\n";
    print_matrix_fp32(C);

    // 释放 tile（可选，通常程序结束时自动释放）
    _tile_release();
}



