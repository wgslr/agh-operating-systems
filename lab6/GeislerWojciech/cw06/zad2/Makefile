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
	-ffloat-store -fno-common \
	-fdiagnostics-color=always \
	$(O) \
#SANITIZER=-fsanitize=address -fsanitize=undefined -fsanitize=leak -lasan -lubsan
.PHONY: all test clean debug

all: server client

server: server.c common.h
	$(CC) $< -o $@ $(CFLAGS) $(SANITIZER)

client: client.c common.h
	$(CC) $< -o $@ $(CFLAGS) $(SANITIZER)

test: server client
	./client < test.in
	./client test.in

clean:
	-rm *.o *.so *.a *.bin *.out server client 
	-find -type p -delete


