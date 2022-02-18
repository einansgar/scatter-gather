#include <stdlib.h>

void generate(int **numbers, const unsigned int amount, unsigned int seed) {
    srand(seed);
    *numbers = (int*) malloc(sizeof(int) * amount);
    for (unsigned int i = 0; i < amount; i++) {
        (*numbers)[i] = rand();
    }
}
