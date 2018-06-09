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
.PHONY: all test clean debug

all: server client

test: server client
	./main config.in > out
	diff <(sort ../iliada.txt | uniq) <(sort out | uniq)

%: %. common.h

clean:
	-rm *.o *.so *.a *.bin *.out server client 
	-find -type p -delete
