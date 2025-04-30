#include <cstddef>
#include <cstdint>
#include <cstring>
#include <immintrin.h>
#include <iostream>
#include <sys/syscall.h>
#include <unistd.h>


size_t MAX = 128;
int MAX_ROWS = 8;
int MAX_COLS = 16;
int STRIDE = 64; // AMX 的内存操作要求 STRIDE 是 64 字节的倍数，最小值为 64
int ARCH_GET_XCOMP_PERM = 0x1022;
int ARCH_REQ_XCOMP_PERM = 0x1023;
int XFEATURE_XTILECFG = 17;
int XFEATURE_XTILEDATA = 18;

// TIlE配置结构体
// AMX（Advanced Matrix Extensions）在硬件上支持 8 个 tile 寄存器，编号从 tmm0
// 到 tmm7。
struct __tile_config {
  uint8_t palette_id; // 配置模式:0 1
  uint8_t start_row;
  uint8_t reserved_0[14];
  uint16_t colsb[16]; // 每个 tile 的列字节数
  
  // 每个 tile 的行数:rows[i] 的确是第 i 个 tile 的行数，范围是 0 到
  // 255（uint8_t 的取值范围） AMX 当前支持 8 个 tile（tmm0 到
  // tmm7），所以只用前 8 个元素，后 8 个预留。 如，rows[0] = 8 表示 tile 0 有 8
  // 行。
  uint8_t rows[16]; // 每个 tile 的行数
};

bool set_tiledata_use() {
  if (syscall(SYS_arch_prctl, ARCH_REQ_XCOMP_PERM, XFEATURE_XTILEDATA)) {
    std::cerr << "Failed to enable AMX tile data\n";
    return false;
  } else {
    printf("\n TILE DATA USE SET - OK \n\n");
    return true;
  }
  return true;
}

void init_tile_config(__tile_config *tileinfo) {

  int i;
  tileinfo->palette_id = 1;
  tileinfo->start_row = 0;

  for (i = 0; i < 1; ++i) {
    tileinfo->colsb[i] = MAX_ROWS;
    tileinfo->rows[i] = MAX_ROWS;
  }

  for (i = 1; i < 4; ++i) {
    tileinfo->colsb[i] = MAX_COLS;
    tileinfo->rows[i] = MAX_ROWS;
  }

  _tile_loadconfig(tileinfo);
}

void matrix_multiply_8x8_int8_amx(const int8_t *A, const int8_t *B,
                                  int32_t *C) {

  __tile_config tile_data = {0};
  if (!set_tiledata_use()) {
    exit(-1);
  }

  init_tile_config(&tile_data);
  // 加载矩阵 A 和 B 到 tile
  _tile_loadd(2, A, STRIDE);
  _tile_loadd(3, B, STRIDE);
  _tile_loadd(1, C, STRIDE);
  // 执行矩阵乘法 : 执行INT8点积，结果累加到f32
  _tile_dpbssd(1, 2, 3);
  // 将结果从 tile 中存储到 C 矩阵
  _tile_stored(1, C, STRIDE);

 
}

/* Initialize int8_t buffer */
static void init_buffer(int8_t *buf, int8_t value) {
  int rows, colsb, i, j;
  rows = MAX_ROWS;
  colsb = MAX_COLS;

  for (i = 0; i < rows; i++)
    for (j = 0; j < colsb; j++) {
      buf[i * colsb + j] = value;
    }
}

/* Initialize int32_t buffer */
static void init_buffer32(int32_t *buf, int32_t value) {
  int rows, colsb, i, j;
  rows = MAX_ROWS;
  colsb = MAX_COLS;
  int colsb2 = colsb / 4;

  for (i = 0; i < rows; i++)
    for (j = 0; j < (colsb2); j++) {
      buf[i * colsb2 + j] = value;
    }
}
/* Print int8_t buffer */
static void print_buffer(int8_t *buf, int32_t rows, int32_t colsb) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < (colsb); j++) {
      printf("%d ", buf[i * colsb + j]);
    }
    printf("\n");
  }
  printf("\n");
}

/* Print int32_t buffer */
static void print_buffer32(int32_t *buf, int32_t rows, int32_t colsb) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < (colsb); j++) {
      printf("%d ", buf[i * colsb + j]);
    }
    printf("\n");
  }
  printf("\n");
}

int main() {

  int8_t A[MAX];
  int8_t B[MAX];
  int32_t C[MAX / 4];
  int rows = MAX_ROWS;
  int colsb = MAX_COLS;

  init_buffer(A, 2);
  print_buffer(A, rows, colsb);

  init_buffer(B, 2);
  print_buffer(B, rows, colsb);

  init_buffer32(C, 0);

  matrix_multiply_8x8_int8_amx(A, B, C);

  print_buffer32(C, rows, colsb / 4);
  _tile_release();
  return 0;
}