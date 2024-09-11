#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
int main()
{
	int fd = open("/dev/paridev",O_RDONLY);
	if(fd == -1) {
		printf("Could not open file\n");
		return -1;
	}
	printf("Opened file\n");
	close(fd);
	printf("Closed file");
	return 0;
}

