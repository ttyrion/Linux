#include <unistd.h>
#include <stdio.h>

int main(int argc, char** argv) {
	//buffered io
	putchar('a');
	//unbuffered io
	write(STDOUT_FILENO, "b", 1);

	putchar('a');
	write(STDOUT_FILENO, "b", 1);

	putchar('a');
	write(STDOUT_FILENO, "b", 1);

	printf("\n");

	return 0;
}

/*
	OUTPUT: bbbaaa
	The putchar() put 'a' into the stream, but not the kenel buffer or disk.
	The three 'a' were flushed to kernel(and then disk) when the process exits(after calling write())
*/
