CC=gcc
O=-O0
override CFLAGS+=-lm -Wall -Wextra -Wall -std=c11 \
	-Wformat=2 \
	-g \
	-Wswitch-default -Wswitch-enum -Wcast-align -Wpointer-arith \
	-Wbad-function-cast -Wstrict-overflow=5 -Wstrict-prototypes -Winline \
	-Wundef -Wnested-externs -Wcast-qual -Wshadow  \
	-Wlogical-op -Wfloat-equal -Wstrict-aliasing=2 -Wredundant-decls \
	-Wold-style-definition -Wuninitialized -Winit-self \
	-ffloat-store -fno-common -fstrict-aliasing \
	-fdiagnostics-color=always \
	$(O) \
#SANITIZER=-fsanitize=address -fsanitize=undefined -fsanitize=leak -lasan -lubsan
.PHONY: all test clean debug

all: main

main: main.c
	$(CC) $< -o $@ $(CFLAGS) $(SANITIZER)

test: main
	./main in.batch

clean:
	-rm *.o *.so *.a *.bin *.out main child

