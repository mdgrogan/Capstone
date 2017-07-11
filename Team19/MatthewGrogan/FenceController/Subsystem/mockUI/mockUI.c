#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define INFILE "../FCtoUI.txt"
#define OUTFILE "../UItoFC.txt"

int main()
{
    int fd;
    struct flock lock;
    char inbuf[256] = {0};
    char outbuf[256] = {0};

    memset(&lock, 0, sizeof(lock));

    while (1) {
	gets(outbuf);

	fd = open(OUTFILE, O_WRONLY);
	lock.l_type = F_WRLCK;
	fcntl(fd, F_SETLKW, &lock);
	write(fd, outbuf, sizeof(outbuf));
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLKW, &lock);
	close(fd);

	fd = open(INFILE, O_RDONLY);
	lock.l_type = F_RDLCK;
	fcntl(fd, F_SETLKW, &lock);
	read(fd, inbuf, sizeof(inbuf));
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLKW, &lock);
	close(fd);

	printf("%s\n", inbuf);
	memset(inbuf, 0, sizeof(inbuf));
	memset(outbuf, 0, sizeof(outbuf));	
    }	
    return 0;
}
