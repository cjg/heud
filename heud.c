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

static void init();
static void cleanup();
static void grab_keyboard(const char *dev);
static void create_virtual_keyboard();
static void emit_event(struct input_event *event);
static int capture_event(struct input_event *event, unsigned type, int retry);
static unsigned int replace_key(unsigned int code, unsigned int *modifier);


int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s /dev/input/<kbd_event>\n", argv[0]);
	}

	init();
	grab_keyboard(argv[1]);
	create_virtual_keyboard();

	int leftalt_pressed = 0;

	struct input_event event;
	unsigned int replacement, modifier;
	int is_press = 0;
	int is_release = 0;
	
	for (;;) {
		capture_event(&event, EV_KEY, 1);
		is_press = event.value == 1;
		is_release = event.value == 0;

		if (!is_press && !is_release) {
			continue;
		}
		
		if (event.code == KEY_LEFTALT) {
			leftalt_pressed = is_press; 
			continue;
		}

		if (leftalt_pressed == 1) {
			replacement = replace_key(event.code, &modifier);
			if (is_press && modifier != 0) {
				event.code = modifier;
				emit_event(&event);
			}
			event.code = replacement;
			emit_event(&event);
			if (is_release && modifier != 0) {
				event.code = modifier;
				emit_event(&event);
			}
			continue;
		} 
		emit_event(&event);

	}
	return 0;
}

static void init() {
	kbd_fd = 1;
	uinput_fd = -1;
	atexit(cleanup);
}

static void cleanup() {
	ioctl(kbd_fd, EVIOCGRAB, NULL);
	close(kbd_fd);

	libevdev_uinput_destroy(virtkbd_dev);
	libevdev_free(kbd_dev);
	close(uinput_fd);
}

static void grab_keyboard(const char *dev) {
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

static void create_virtual_keyboard() {
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

static void emit_event(struct input_event *event) {
	libevdev_uinput_write_event(virtkbd_dev,
				    event->type,
				    event->code,
				    event->value);
	libevdev_uinput_write_event(virtkbd_dev,
				    EV_SYN,
				    SYN_REPORT,
				    0);
}

static int capture_event(struct input_event *event, unsigned type, int retry) {
	for (;;) {
		read(kbd_fd, event, sizeof(struct input_event));
		if (event->type == type) {
			return 0;
		}
		if (!retry) {
			return -1;
		}
	}
}

static unsigned int replace_key(unsigned int code, unsigned int *modifier) {
	*modifier = 0;
	switch(code) {
	case KEY_LEFT:
		return KEY_HOME;
	case KEY_RIGHT:
		return KEY_END;
	case KEY_UP:
		return KEY_PAGEUP;
	case KEY_DOWN:
		return KEY_PAGEDOWN;
	case KEY_X:
		return KEY_CUT;
	case KEY_C:
		return KEY_COPY;
	case KEY_V:
		return KEY_PASTE;
	}
	*modifier = KEY_LEFTALT;
	return code;
}
