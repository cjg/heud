.PHONY: all clean
all: heud

heud: heud.c
	gcc $(shell pkg-config libevdev --cflags) -o heud heud.c $(shell pkg-config libevdev --libs)

clean:
	@rm -f heud
