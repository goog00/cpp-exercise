### **编译步骤**

1. **编译汇编文件**：
   
   nasm -f macho64 gemv_kernel.asm -o gemv_kernel.o
   
3. **编译并链接**：
  
   g++ -O3 -mavx2 -mfma -std=c++11 gemv.cpp gemv_kernel.o -o gemv_test
   
5. **运行**：
   
   `./gemv_test
