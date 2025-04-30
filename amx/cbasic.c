#include <immintrin.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ROWS 8
#define MAX_COLS 8
#define STRIDE 64

struct __tile_config {
    uint8_t palette_id;
    uint8_t start_row;
    uint8_t reserved[14];
    uint8_t rows[16];
    uint16_t colsb[16];
};

static int set_tiledata_use() {
    if (syscall(SYS_arch_prctl, 0x1023, 18)) {
        fprintf(stderr, "Failed to enable AMX tile data\n");
        return 0;
    }
    printf("AMX tile data enabled\n");
    return 1;
}

static void init_tile_config(struct __tile_config* tileinfo) {
    tileinfo->palette_id = 1;
    tileinfo->start_row = 0;
    tileinfo->rows[2] = MAX_ROWS;
    tileinfo->colsb[2] = MAX_COLS;
    tileinfo->rows[3] = MAX_ROWS;
    tileinfo->colsb[3] = MAX_COLS;
    tileinfo->rows[1] = MAX_ROWS;
    tileinfo->colsb[1] = MAX_COLS * sizeof(int32_t);
    _tile_loadconfig(tileinfo);
}

static void matrix_multiply_8x8_int8_amx(const int8_t* A, const int8_t* B, int32_t* C) {
    __attribute__((aligned(64))) struct __tile_config tile_data = {0};
    if (!set_tiledata_use()) {
        exit(-1);
    }
    init_tile_config(&tile_data);
    _tile_loadd(2, A, STRIDE);
    _tile_loadd(3, B, STRIDE);
    _tile_loadd(1, C, STRIDE);
    _tile_dpbssd(1, 2, 3);
    _tile_stored(1, C, STRIDE);
}

static void print_matrix_int8(const int8_t* mat) {
    for (int i = 0; i < MAX_ROWS; ++i) {
        for (int j = 0; j < MAX_COLS; ++j) {
            printf("%d ", mat[i * MAX_COLS + j]);
        }
        printf("\n");
    }
    printf("\n");
}

static void print_matrix_int32(const int32_t* mat) {
    for (int i = 0; i < MAX_ROWS; ++i) {
        for (int j = 0; j < MAX_COLS; ++j) {
            printf("%d ", mat[i * MAX_COLS + j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main() {
    __attribute__((aligned(64))) int8_t A[MAX_ROWS * MAX_COLS];
    __attribute__((aligned(64))) int8_t B[MAX_ROWS * MAX_COLS];
    __attribute__((aligned(64))) int32_t C[MAX_ROWS * MAX_COLS];

    for (int i = 0; i < MAX_ROWS * MAX_COLS; ++i) {
        A[i] = (int8_t)i;
        B[i] = (int8_t)i;
        C[i] = 0;
    }

    printf("Matrix A:\n");
    print_matrix_int8(A);
    printf("Matrix B:\n");
    print_matrix_int8(B);

    matrix_multiply_8x8_int8_amx(A, B, C);

    printf("Result Matrix C (INT32):\n");
    print_matrix_int32(C);

    _tile_release();
    return 0;
}