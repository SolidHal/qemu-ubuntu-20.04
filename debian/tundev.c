/* (C) 2004 Rusty Russell.  Licenced under the GNU GPL. */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <net/if.h>
#include <linux/if_tun.h>

/* Tiny code to open tap/tun device, and hand the fd to qemu.  Run
   setuid root, drops privs. */
int main(int argc, char *argv[])
{
	struct ifreq ifr;
	int fd;

	if (argc == 1) {
		fprintf(stderr, "Usage: tundev qemu <qemu options>...\n");
		exit(1);
	}

	fd = open("/dev/net/tun", O_RDWR);
	if (fd < 0) {
		perror("Could not open /dev/net/tun");
		exit(1);
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	strncpy(ifr.ifr_name, "tun%d", IFNAMSIZ);
	if (ioctl(fd, TUNSETIFF, (void *) &ifr) != 0) {
		perror("Could not get tun device");
		exit(1);
	}
	printf("Got tun device %s on fd %i\n", ifr.ifr_name, fd);

	/* Drop effective uid. */
	setuid(getuid());

	/* Insert -tun-fd=3 arg. */
	argv[0] = argv[1];
	asprintf(&argv[1], "-tun-fd=%d", fd);
	execvp(argv[0], argv);
	perror("Exec of qemu failed");
	exit(1);
}
