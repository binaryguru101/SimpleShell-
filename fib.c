#include <stdio.h>
#include <stdlib.h>

int fib(int n) {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

int main() {
    int result = fib(10);
    printf("%d\n", result); 

    return 0;
}
