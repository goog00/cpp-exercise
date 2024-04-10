#include <iostream>
using namespace std;

void swap(int &x,int &y);

int main(){
    int a = 100;
    int b = 200;

    cout << "change before ,value of a :" << a << endl;
    cout << "change before ,value of b :" << b << endl;


    swap(a,b);

    cout << "change before ,value of a " << a << endl;
    cout << "change before ,value of b: " << b << endl;
    return 0;

}

void swap(int & x, int & y){
    int temp;
    temp = x;
    x= y;
    y = temp;
    return ;

}