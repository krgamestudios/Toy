//https://www.programiz.com/c-programming/online-compiler/
#include <stdio.h>

static unsigned int hashUInt(unsigned int x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

int main(void) {
    //find the first number with a specific hash, then print the c-code
    for (unsigned int h = 0; h < 20; h++) {
        for (unsigned int i = 0; i < 100; i++) {
            if (hashUInt(i) % 32 == h) {
                printf("Toy_insertTable(&table, TOY_VALUE_TO_INTEGER(%d), TOY_VALUE_TO_INTEGER(42)); //hash: %d\n", i, h);
                break;
            }
        }
    }

    return 0;
}
