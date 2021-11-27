#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <unistd.h>

int main(int agc, char **argv) {
	int fd;
	struct input_event event;

	fd = open(argv[1], O_RDONLY);
	for (;;) {
		read(fd, &event, sizeof(event));
		printf("<%ld.%06ld> type: 0x%x code: %d value: %d\n",
		       event.time.tv_sec, event.time.tv_usec,
		       event.type, event.code, event.value);
	}
}
hi
