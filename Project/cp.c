#include "type.h"

void cp_file(char *src, char *dst){
	int fd, gd;
	int n;
	char buf[1024];

	fd = open_file(src, "0");
	gd = open_file(dst, "1");
	n = 0;

	while(n = read_file(fd, buf, BLKSIZE)){
		write_file(gd, buf, n);
	}
}
