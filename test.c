#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "cdata_ioctl.h"

int main(int argc, char *argv[])
{
	int fd;
	pid_t pid;
	char buf[64];

	pid = fork();
	if (pid == 0) {
		/* parent */
		sprintf(buf, "[%d]I'm parent", getpid());
	} else if (pid > 0) {
		/* child */
		sprintf(buf, "[%d]I'm child", getpid());
	} else {
		printf("fork error");
		return -1;
	}
	

	fd = open("/dev/cdata", O_RDWR);
	if (fd == -1) {
		printf("open /dev/cdata error.");
		return -1;
	}

	write(fd, buf, sizeof(buf));
	ioctl(fd, IOCTL_SYNC, 0);
	ioctl(fd, IOCTL_EMPTY, 0);
	ioctl(fd, IOCTL_SYNC, 0);

	close(fd);
	return 0;
}
