CC=gcc
O=-O0
override CFLAGS+=-lm -lrt -Wall -Wextra -Wall -std=c11 \
	-Wformat=2 \
	-g \
	-Wswitch-default -Wswitch-enum -Wcast-align -Wpointer-arith \
	-Wbad-function-cast -Wstrict-overflow=5 -Wstrict-prototypes -Winline \
	-Wundef -Wnested-externs -Wcast-qual -Wshadow  \
	-Wlogical-op -Wfloat-equal -Wredundant-decls \
	-Wold-style-definition -Wuninitialized -Winit-self \
	-fdiagnostics-color=always \
	$(O) \
#SANITIZER=-fsanitize=address -fsanitize=undefined -fsanitize=leak -lasan -lubsan
.PHONY: all test clean debug

all: main

barber: barber.c
	$(CC) $^ -o $@ $(CFLAGS) $(SANITIZER)

clean:
	-rm *.o *.so *.a *.bin *.out main 
	-find -type p -delete

%.png: %.pgm
	convert $< -compress png:$@

%.pgm: %.png
	convert $< -compress none pgm:$@

