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

all: barber client

barber: barber.c common.c common.h
	$(CC) $^ -o $@ $(CFLAGS) $(SANITIZER)

client: client.c common.c common.h
	$(CC) $^ -o $@ $(CFLAGS) $(SANITIZER)

test: barber client
	./barber 4 &
	./client 5 5

clean:
	-rm *.o *.so *.a *.bin *.out barber client 
	-find -type p -delete


