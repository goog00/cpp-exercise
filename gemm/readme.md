### **1. main_gemv_kernel_opt0 编译步骤**

1. **编译汇编文件**：

   nasm -f macho64 gemv_kernel_opt0.asm -o gemv_kernel.o
2. **编译并链接**：

   g++ -O3 -mavx2 -mfma -std=c++11 main_gemv_kernel_opt0.cpp  gemv_kernel.o -o gemv_test
3. **运行**：

   ./gemv_test

### **2. main_gemv_kernel_opt1/ main_gemv_kernel_opt2 优化方案**

#### **1. 内存对齐优化**

* **目标**：使用 **vmovaps**（对齐加载）替代 **vmovups**，减少内存访问延迟。
* **前提**：需要在 C++ 中分配对齐内存（例如用 **std::aligned\_alloc**）。
* **改进**：假设输入数据已按 32 字节对齐。

#### **2. 循环展开和寄存器阻塞**

* **目标**：展开内层循环，一次处理更多元素（例如 16 或 32 个），并使用更多寄存器累加。
* **好处**：减少循环分支开销，提高指令级并行性。

#### **3. 多线程支持**

* **目标**：将外层循环（行）分配给多个线程，利用 6 核并行。
* **实现**：在 C++ 中用 OpenMP 调用汇编内核，汇编保持单线程逻辑。

#### **4. 水平加和优化**

* **目标**：用更高效的方式将 256 位向量求和。
* **方法**：用 **vextractf128** 和 **vhaddps** 简化步骤。

#### **5. 边界处理**

* **目标**：支持任意列数 n n **n**，避免遗漏计算。
* **方法**：添加清理循环处理剩余元素。


### 3. main_gemv_kernel_opt1

编译步骤：

1.编译汇编文件

nasm -f macho64 gemv_kernel_opt1.asm -o gemv_kernel.o

2.编译并链接

g++ -O3 -mavx2 -mfma -std=c++11 main_gemv_kernel_opt1.cpp  gemv_kernel.o -o gemv_test

3.运行

./gemv_test

### 4. main_gemv_kernel_opt2

编译步骤：

1.编译汇编文件

nasm -f macho64 gemv_kernel_opt2.asm -o gemv_kernel.o

2.编译并链接

g++ -O3 -mavx2 -mfma -std=c++11 main_gemv_kernel_opt2.cpp  gemv_kernel.o -o gemv_test

3.运行

./gemv_test
