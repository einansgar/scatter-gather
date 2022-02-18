#include <stdio.h>
#include <time.h>
#include "include/random.h"

int main() {
    unsigned int seed = time(NULL);

    int *numbers;
    const unsigned int amount = 100;
    printf("Generate random numbers!\n");

    generate(&numbers, amount, seed);

    printf("The numbers are: \n");

    for (unsigned int i = 0; i < amount; i++) {
        printf("%d\t", numbers[i]);
    }
    printf("\n");
    free(numbers);
    
}