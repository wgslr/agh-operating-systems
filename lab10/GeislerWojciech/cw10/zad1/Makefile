CC=gcc
O=-O0
override CFLAGS+=-lm -lpthread -lrt -Wall -Wextra -Wall -std=c11 \
	-Wformat=2 -g \
	-Wswitch-default -Wcast-align -Wpointer-arith \
	-Wbad-function-cast -Wstrict-overflow=5 -Wstrict-prototypes -Winline \
	-Wundef -Wnested-externs -Wcast-qual -Wshadow  \
	-Wlogical-op -Wfloat-equal -Wredundant-decls \
	-Wold-style-definition -Wuninitialized -Winit-self \
	-fdiagnostics-color=always \
	$(O)
SANITIZER=-fsanitize=address -fsanitize=undefined -fsanitize=leak -lasan -lubsan
PORT=5010
SOCKET=s
.PHONY: all test clean debug

all: server client

test: server client
	./server $(PORT) $(SOCKET) &
	for i in {1..10}; do ./client c$$i UNIX $(SOCKET) & done
	for i in {1..10}; do ./client c$$i INET 127.0.0.1:$(PORT) & done

%: %.c common.h
	$(CC) $^ -o $@ $(CFLAGS) $(SANITIZER)

clean:
	-rm *.o *.so *.a *.bin *.out server client 
	-find -type p -delete
