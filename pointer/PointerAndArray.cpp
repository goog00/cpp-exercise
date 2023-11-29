#include <iostream>
 
using namespace std;
const int MAX = 3;
 
int main ()
{
   int  var[MAX] = {10, 100, 200};

   //int *ptr[MAX] 一个整数指针数组 ptr, ptr的类型是int[]
   // ptr 是一个指针数组，这意味着 ptr[i] 是一个指针，
   //它指向一个整数，而不是一个整数数组。


   //定义一个指针数组 ptr，包含 3 个指针;ptr 的类型为int*
   int*  ptr[MAX];
 
   for (int i = 0; i < MAX; i++)
   {
      ptr[i] = &var[i]; 
   }
   for (int i = 0; i < MAX; i++)
   {
      cout << "Value of var[" << i << "] = ";
      cout << *ptr[i] << endl;
   }
   return 0;
}