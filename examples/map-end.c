#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <unistd.h>
#include <libevdev/libevdev-uinput.h>

int main(int agc, char **argv) {
	struct input_event event;

	int kbd_fd;
	int uinput_fd;

	struct libevdev *kbd_dev;
	struct libevdev_uinput *virtkbd_dev;

	const char *virtkbd_path;
	
	kbd_fd = open(argv[1], O_RDONLY);
	libevdev_new_from_fd(kbd_fd, &kbd_dev);

	uinput_fd = open("/dev/uinput", O_RDWR);	
	libevdev_uinput_create_from_device(kbd_dev,
					   uinput_fd,
					   &virtkbd_dev);

	virtkbd_path = libevdev_uinput_get_devnode(virtkbd_dev);
	printf("Virtual keyboard device: %s\n",
	       virtkbd_path);

	int leftalt_down = 0;
	for (;;) {
		read(kbd_fd, &event, sizeof(event));
		if (event.type != EV_KEY) {
			continue;
		}
		if (event.code == KEY_LEFTALT) {
			leftalt_down = event.value;
			printf("leftalt: %d\n", leftalt_down);
			continue;
		}
		if (leftalt_down && event.code == KEY_RIGHT) {
			event.code = KEY_END;
		}
		libevdev_uinput_write_event(virtkbd_dev,
					    event.type,
					    event.code,
					    event.value);
		libevdev_uinput_write_event(virtkbd_dev,
					    EV_SYN,
					    SYN_REPORT,
					    0);
	}
}
