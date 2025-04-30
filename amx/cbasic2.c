#include <immintrin.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX 1024
#define MAX_ROWS 16
#define MAX_COLS 64
#define STRIDE 64
#define ARCH_GET_XCOMP_PERM 0x1022
#define ARCH_REQ_XCOMP_PERM 0x1023
#define XFEATURE_XTILECFG 17
#define XFEATURE_XTILEDATA 18

// typedef struct __tile_config 
// {
//   uint8_t palette_id;
//   uint8_t start_row;
//   uint8_t reserved_0[14];
//   uint8_t rows[16];
//   uint16_t colsb[16];
// } __tilecfg;

typedef struct __tile_config
{
  uint8_t palette_id;
  uint8_t start_row;
  uint8_t reserved_0[14];
  uint16_t colsb[16]; 
  uint8_t rows[16]; 
  
} __tilecfg;

static bool set_tiledata_use() {
  if (syscall(SYS_arch_prctl, ARCH_REQ_XCOMP_PERM, XFEATURE_XTILEDATA)) {
    fprintf(stderr, "Failed to enable AMX tile data\n");
    return false;
  } else
  {
     printf("\n TILE DATA USE SET - OK \n\n");
     return true;
  }
  return true;
}

static void init_tile_config(__tilecfg *tileinfo) {
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

static void matrix_multiply_8x8_int8_amx(const int8_t *A, const int8_t *B, int32_t *C) {
   
  __tilecfg tile_data = {0};
 
  if (!set_tiledata_use())
    exit(-1);


  init_tile_config(&tile_data);
 
  _tile_loadd(2, A, STRIDE);
  _tile_loadd(3, B, STRIDE);
  _tile_loadd(1, C, STRIDE);
  _tile_dpbssd(1, 2, 3);
  _tile_stored(1, C, STRIDE);
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
  int32_t C[MAX/4];
  int rows  = MAX_ROWS;
  int colsb = MAX_COLS;



  init_buffer(A, 2);
  print_buffer(A, rows, colsb);

  init_buffer(B, 2);
  print_buffer(B, rows, colsb);

  // Init dst matrix buffers with data
  init_buffer32(C, 0);

  matrix_multiply_8x8_int8_amx(A, B, C);

  printf("Result Matrix C (INT32):\n");
  print_buffer32(C, rows, colsb / 4);

  _tile_release();
  return 0;
}