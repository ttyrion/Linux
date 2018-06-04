#include "stdio.h"

void print_mem(char* start, int bytes);

//Most of the times compiler takes care of endianness, 
//however, endianness becomes an issue in network programming!
//Because standard byte order for networks is big endian, also known as "network byte order".

/*
Once I was confused: 
Would the growing direction of stack address (growing upwards or downwards, platform dependent) affect the result of the following code?
The answer is supposed to be no.
1. The growing direction of stack address affects the order of two objects in the address space.
2. The endian type affects the internal bytes order stored of that one object.
*/

int main(int argc, char* argv[]) {
	int i = 0x01020304;
	//print_mem((char*)&i, sizeof(i));
	
	char* p = (char*)&i;
	char c = *p;
	if(c == 01) {
		printf("big endian.\n");
	}
	else if(c == 04) {
		printf("little endian.\n");
	}
	
	return 0;
}

void print_mem(char* start, int bytes) {
	for(int i = 0; i < bytes; ++i) {
		printf("%.2x", start[i]);
	}
	printf("\n");
}



