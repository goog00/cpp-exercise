// 一个返回指针的函数，如下所示：

// int * myFunction()
// {
// .
// }

#include <iostream>
#include <ctime>
#include <cstdlib>

using namespace std;

int * getRandom() {
    static int r[10];

    srand((unsigned)time( NULL));

    for(int i = 0; i < 10 ; ++i){
        r[i] = rand();
        cout << r[i] << endl;
    }

    return r;

}


int main(){
    int *p;

    p = getRandom();
    for(int i = 0; i < 10; i++){
        cout << "*(p + " << i << ") : ";
        cout << *(p + i) << endl;
    }
    return 0;

}