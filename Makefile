heud: heud.c
	gcc -o heud heud.c

.PHONY: clean
clean:
	@rm -f heud
