### **编译步骤**

1. **编译汇编文件**：
   **bash**

   **Collapse**Wrap**Copy**

   `<span>nasm -f macho64 gemv_kernel.asm -o gemv_kernel.o</span>`
2. **编译并链接**：
   **bash**

   **Collapse**Wrap**Copy**

   `<span>g++ -O3 -mavx2 -mfma -std=c++11 gemv.cpp gemv_kernel.o -o gemv_test</span>`
3. **运行**：
   **bash**

   **Collapse**Wrap**Copy**

   `<span>./gemv_test</span>`
