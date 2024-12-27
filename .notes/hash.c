#include <stdio.h>
#include <stdint.h>

uint32_t hash (uint32_t x) {
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x);
	return x;
}

uint32_t unhash ( uint32_t x ) {
	x = (( x >> 16) ^ x) * 0x119de1f3;
	x = (( x >> 16) ^ x) * 0x119de1f3;
	x = (( x >> 16) ^ x);
	return x;
}

int main() {
	//I legit didn't know this algorithm could be reversed. Neat.
	uint32_t value = 42;
	printf("%u %u %u", value, hash(value), unhash(hash(value)));
	return 0;
}