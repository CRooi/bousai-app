#include <stdio.h>

int factorial(int n){
    if (n == 0) return 1;
    //递归调用
    return n * factorial(n - 1);
}

int main(){
    int N;
    printf("input a number: ");
    scanf("%d", &N);
    int result = factorial(N);
    printf("阶乘：%d\n", result);
    return 0;
}