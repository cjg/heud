#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <libevdev/libevdev-uinput.h>
#include <stdlib.h>

int kbd_fd;
struct libevdev *kbd_dev;

int uinput_fd;
struct libevdev_uinput *virtkbd_dev;

void init();
void finalize();
void grab_keyboard(const char *dev);
void create_virtual_keyboard();
void emit_event(struct input_event *event);
void emit_events(struct input_event *event, int len);
void capture_key_event(struct input_event *events);

const char *event_type(unsigned short type);


void emit(int fd, struct input_event ie) {
	write(fd, &ie, sizeof(ie));
	fsync(fd);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s /dev/input/<kbd_event>\n", argv[0]);
	}

	init();
	grab_keyboard(argv[1]);
	create_virtual_keyboard();

	int leftalt_pressed = 0;

	struct input_event events[3];
	for (;;) {
 		capture_key_event(events);
		if (events[1].code == KEY_LEFTALT) {
			if (events[1].value == 0) {
				leftalt_pressed = 0;
				printf("Alt_L released\n");
			} else if (leftalt_pressed == 0) {
				leftalt_pressed = 1;
				printf("Alt_L pressed\n");
			}
		}
		if (leftalt_pressed == 1) {
			if (events[1].code == KEY_LEFT) {
				printf("Replacing LEFT with HOME\n");
				events[1].code = KEY_HOME;
			}
		}
		if (events[1].code != KEY_LEFTALT) {
			emit_events(events, 3);
		}
	}
	return 0;
}

void init() {
	kbd_fd = 1;
	uinput_fd = -1;
	atexit(finalize);
}

void finalize() {
	printf("Finalizing\n");

	ioctl(kbd_fd, EVIOCGRAB, NULL);
	close(kbd_fd);
}

void grab_keyboard(const char *dev) {
	kbd_fd = open(dev, O_RDWR);
	if (kbd_fd < 0) {
		fprintf(stderr, "Error opening keyboard at '%s'\n", dev);
		exit(-1);
	}
	
	int grab = 1;
	if (ioctl(kbd_fd, EVIOCGRAB, &grab) != 0) {
		fprintf(stderr, "Error setting exclusive mode to keyboard\n");
		exit(-1);
	}

	if (libevdev_new_from_fd(kbd_fd, &kbd_dev) != 0) {
		fprintf(stderr, "Error creating evdev device for keyboard\n");
		exit(-1);
	}
}

void create_virtual_keyboard() {
	uinput_fd = open("/dev/uinput", O_RDWR);
	if (uinput_fd < 0) {
		fprintf(stderr, "Error opening /dev/uinput\n");
		exit(-1);
	}

	if (libevdev_uinput_create_from_device(kbd_dev, uinput_fd,
					       &virtkbd_dev) != 0) {
		fprintf(stderr, "Error creating virtual keyboard\n");
	}
}

void emit_event(struct input_event *event) {
	libevdev_uinput_write_event(virtkbd_dev,
				    event->type,
				    event->code,
				    event->value);
}

void emit_events(struct input_event *event, int len) {
	int i;
	for (i = 0; i < len; i++) {
		emit_event(event + i);
	}
}

int capture_event(struct input_event *event, unsigned type, int retry) {
	for (;;) {
		read(kbd_fd, event, sizeof(struct input_event));
		if (event->type == type) {
			return 0;
		}
		// unexpected event just passing it away
		emit_event(event);
		if (!retry) {
			return -1;
		}
	}
}


void capture_key_event(struct input_event *events) {
	for (;;) {
		capture_event(events, EV_MSC, 1);
		if (capture_event(events + 1, EV_KEY, 0) != 0) {
			emit_event(events);
			continue;
		}
		if (capture_event(events + 2, EV_SYN, 0) != 0) {
			emit_events(events, 2);
			continue;
		}
		break;
	}
}
