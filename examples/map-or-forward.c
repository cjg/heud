#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <unistd.h>
#include <libevdev/libevdev-uinput.h>

void send_key_event(struct libevdev_uinput *dev,
		    unsigned int code, int value) {
	libevdev_uinput_write_event(dev,
				    EV_KEY,
				    code,
				    value);
	libevdev_uinput_write_event(dev,
				    EV_SYN,
				    SYN_REPORT,
				    0);

}

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

	usleep(100000);
	int grab = 1;
	ioctl(kbd_fd, EVIOCGRAB, &grab);

	int leftalt_down = 0;
	for (;;) {
		read(kbd_fd, &event, sizeof(event));
		if (event.type != EV_KEY || event.value > 1) {
			continue;
		}
		if (event.code == KEY_LEFTALT) {
			leftalt_down = event.value;
		}
		if (leftalt_down && event.code == KEY_RIGHT) {
			if (event.value == 1) {
				send_key_event(virtkbd_dev,
					       KEY_LEFTALT,
					       0);
				event.code = KEY_END;
			} else if (event.value == 0) {
				send_key_event(virtkbd_dev,
					       KEY_END,
					       event.value);
				event.code = KEY_LEFTALT;
				event.value = 1;
			}
		}
		send_key_event(virtkbd_dev, event.code, event.value);
	}
}
