CC=gcc
O=-O0
CFLAGS=-lm -Wall -Wextra -Wall \
	-Wno-missing-braces -Wextra -Wno-missing-field-initializers -Wformat=2 \
	-Wswitch-default -Wswitch-enum -Wcast-align -Wpointer-arith \
	-Wbad-function-cast -Wstrict-overflow=5 -Wstrict-prototypes -Winline \
	-Wundef -Wnested-externs -Wcast-qual -Wshadow -Wunreachable-code \
	-Wlogical-op -Wfloat-equal -Wstrict-aliasing=2 -Wredundant-decls \
	-Wold-style-definition \
	-fno-omit-frame-pointer -ffloat-store -fno-common -fstrict-aliasing \
	$(O)
SANITIZER=
.PHONY: all test clean debug

all: main child

main: main.c
	$(CC) $< -o $@ $(CFLAGS) $(SANITIZER)

child: child.c
	$(CC) $< -o $@ $(CFLAGS) $(SANITIZER)

clean:
	-rm *.o *.so *.a *.bin *.out main big long

