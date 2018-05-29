CC=gcc
O=-O0
override CFLAGS+=-lm -lpthread -lrt -Wall -Wextra -Wall -std=c11 \
	-Wformat=2 \
	-g \
	-Wswitch-default -Wswitch-enum -Wcast-align -Wpointer-arith \
	-Wbad-function-cast -Wstrict-overflow=5 -Wstrict-prototypes -Winline \
	-Wundef -Wnested-externs -Wcast-qual -Wshadow  \
	-Wlogical-op -Wfloat-equal -Wredundant-decls \
	-Wold-style-definition -Wuninitialized -Winit-self \
	-fdiagnostics-color=always \
	$(O)
#SANITIZER=-fsanitize=address -fsanitize=undefined -fsanitize=leak -lasan -lubsan
.PHONY: all test clean debug

all: main

main: main.c
	$(CC) $^ -o $@ $(CFLAGS) $(SANITIZER)

clean:
	-rm *.o *.so *.a *.bin *.out main 
	-find -type p -delete

test: main
	mkdir -p out
	echo "threads;time;img;filter" > Times.txt
	bash -c 'for I in {3..65}; do for T in {1,2,4,8}; do ./main $$T input600.pgm filters/filter$$I.in out/out.$$T.$$I.pgm >> Times.txt; done; done'

test2: main
	mkdir -p out2
	echo "threads;time;img;filter" > Times2.txt
	bash -c 'for I in {3..65}; do for T in {1..16}; do ./main $$T input600.pgm filters/filter$$I.in out2/out.$$T.$$I.pgm >> Times2.txt; done; done'
%.png: %.pgm
	convert $< png:$@

%.pgm: %.png
	convert $< -compress none pgm:$@
