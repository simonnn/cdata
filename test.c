#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "cdata_ioctl.h"

int main(int argc, char *argv[])
{
	int fd;

	fd = open("/dev/cdata", O_RDONLY);
	if (fd == -1) {
		printf("open /dev/cdata error.");
		return -1;
	}

	printf("ioctl num:%d\n", IOCTL_EMPTY);
	ioctl(fd, IOCTL_SYNC, 0);

	close(fd);
	return 0;
}
