#include <arm_neon.h>


// 计算两个浮点向量的点积（长度为 4）
float dot_product_float(float* a, float* b) {
    // 加载向量
    float32x4_t va = vld1q_f32(a);  // 加载 a[0:4]
    float32x4_t vb = vld1q_f32(b);  // 加载 b[0:4]

    // 逐元素相乘
    float32x4_t prod = vmulq_f32(va, vb);  // prod[i] = a[i] * b[i]

    // 归约求和
    float result = vaddvq_f32(prod);  // result = prod[0] + prod[1] + prod[2] + prod[3]

    return result;
}

