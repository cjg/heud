KERNEL_VERSION := $(shell uname -r)
KERNEL_HEADERS := /lib/modules/$(KERNEL_VERSION)/source/include

.PHONY: all
all: heud

key_codes.c: key_codes
	@echo '#include <uapi/linux/input-event-codes.h>' > key_codes.c
	@echo 'const char *key_code(unsigned short code) {' >> key_codes.c
	@for i in $(shell cat key_codes); do echo 'if (code == 'KEY_$$i')' \{ return \"$$i\"\; \}; done >> key_codes.c
	@echo return \"???\"\; >> key_codes.c
	@echo '}' >> key_codes.c

key_codes:
	grep "#define KEY_" $(KERNEL_HEADERS)/uapi/linux/input-event-codes.h | sed -e 's|#define KEY_||' -e 's|[^A-Z0-9_].*||' | sort | uniq > key_codes

heud: heud.c key_codes.c
	gcc -I/lib/modules/$$(uname -r)/source/include -o heud heud.c key_codes.c

.PHONY: clean
clean:
	@rm -f heud key_codes key_codes.c
