#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
int main(void) {
	char ch;
	read(STDIN_FILENO, &ch, 1);
if (lseek(STDIN_FILENO, -1, SEEK_CUR) == -1) printf("Cannot seek on stdin!\n");
}
