#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
/* for mmap() */
#include <sys/mman.h>
#include "cdata_ioctl.h"

int main(int argc, char *argv[])
{
	int fd;
	pid_t pid;
	char buf[64];
	char p[4] = {0x00, 0x00, 0xff, 0x00};	
	unsigned char *fb;
	int i;

#if 0
	pid = fork();
	if (pid > 0) {
		/* parent */
		sprintf(buf, "[%d]I'm parent", getpid());
	} else if (pid == 0) {
		/* child */
		sprintf(buf, "[%d]I'm child", getpid());
	} else {
		printf("fork error");
		return -1;
	}
#endif

	fd = open("/dev/cdata", O_RDWR);
	if (fd == -1) {
		printf("open /dev/cdata error.");
		return -1;
	}

#if 0
	/* basic test */
	write(fd, buf, sizeof(buf));
#endif

	/* mmap() */
	fb = (unsigned char *)mmap(0, 240*320*4, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, 0);
	for (i = 0; i < 500; i++) {
		*fb = 0xff; fb++;
		*fb = 0x00; fb++;
		*fb = 0x00; fb++;
		*fb = 0x00; fb++;
	}

#if 0
	/* ioremap, kernel timer */
	while (1) {
		write(fd, p, 4);
	}
#endif

#if 0
	/* ioctl */
	ioctl(fd, IOCTL_SYNC, 0);
	ioctl(fd, IOCTL_EMPTY, 0);
	ioctl(fd, IOCTL_SYNC, 0);
#endif

	close(fd);
	return 0;
}
