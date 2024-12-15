//https://www.programiz.com/c-programming/online-compiler/
#include <stdio.h>

static unsigned int hashUInt(unsigned int x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

int main(void) {
    //print the index/hash pairs
    for (unsigned int i = 0; i < 100; i++) {
        unsigned int h = hashUInt(i);
        printf("%u: %u %% 8 = %u\n", i, h, h % 8);
    }

    return 0;
}
