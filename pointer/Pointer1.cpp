#include <iostream>
using namespace std;

//能接受指针作为参数的函数，也能接受数组作为参数

double getAverage(int *arr,int size);

int main(){

    int balance[5] = {1000,2,3,17,50};

    double avg;

    avg = getAverage(balance,5);

    cout << "Average value is :" << avg << endl;

    return 0;

}

double getAverage(int *arr,int size){
    int i , sum = 0;
    double avg ;

    for(i = 0; i < size; ++i){
        sum += arr[i];

    }

    avg  = double(sum) / size;

    return avg;
}