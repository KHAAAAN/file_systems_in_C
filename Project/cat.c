#include "type.h"

void cat_file(char *pathname){
	char buf[2048], dummy = 0;
	int n;
	int fd;

	fd = open_file(pathname, "0");

	while(n = read_file(fd, buf, 1024 )){
		buf[n] = 0;
		printf("%s", buf); 
	}
	putchar('\n');
	close_file(atoi(pathname));
}
