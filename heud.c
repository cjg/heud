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

unsigned map_code(unsigned code, int left_down) {
	if (!left_down) {
		return code;
	}
	switch (code) {
	case KEY_LEFT:
		return KEY_HOME;
	case KEY_RIGHT:
		return KEY_END;
	case KEY_UP:
		return KEY_PAGEUP;
	case KEY_DOWN:
		return KEY_PAGEDOWN;
	case KEY_C:
		return KEY_COPY;
	case KEY_V:
		return KEY_PASTE;
	}
	return code;
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

	int mapped_code = 0;
	int mappable = 0;
	
	for (;;) {
		read(kbd_fd, &event, sizeof(event));
		if (event.type != EV_KEY || event.value > 1 ||
		    event.code == KEY_CAPSLOCK) {
			continue;
		}
		if (event.code == KEY_LEFTALT) {
			leftalt_down = event.value;
		}
		mappable = event.code == KEY_LEFT ||
			event.code == KEY_RIGHT ||
			event.code == KEY_UP ||
			event.code == KEY_DOWN ||
			event.code == KEY_C ||
			event.code == KEY_V;
		
		if (mappable) {
			if (event.value == 0) {
				// release
				send_key_event(virtkbd_dev,
					       mapped_code,
					       0);
				if (leftalt_down) {
					send_key_event(virtkbd_dev,
						       KEY_LEFTALT,
						       1);
				}
				continue;
			}
			// press
			mapped_code = map_code(event.code, leftalt_down);
			if (leftalt_down) {
				send_key_event(virtkbd_dev,
					       KEY_LEFTALT,
					       0);
			}
			event.code = mapped_code;
		}
		send_key_event(virtkbd_dev, event.code, event.value);
	}
}
