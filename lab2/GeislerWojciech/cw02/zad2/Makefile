CC=gcc
RESULTS=times.txt
O=-O2
CFLAGS=-Wall -Wextra -fPIC  -Wall \
	-Wno-missing-braces -Wextra -Wno-missing-field-initializers -Wformat=2 \
	-Wswitch-default -Wswitch-enum -Wcast-align -Wpointer-arith \
	-Wbad-function-cast -Wstrict-overflow=5 -Wstrict-prototypes -Winline \
	-Wundef -Wnested-externs -Wcast-qual -Wshadow -Wunreachable-code \
	-Wlogical-op -Wfloat-equal -Wstrict-aliasing=2 -Wredundant-decls \
	-Wold-style-definition -Werror \
	-fno-omit-frame-pointer -ffloat-store -fno-common -fstrict-aliasing \
	$(O)
.PHONY: test clean

main: main.c
	$(CC) $< -o $@ $(CFLAGS)

test: main
	./main example '<' 2018-03-18 | tee smaller.out
	echo
	./main example '=' 2018-03-18 | tee equal.out
	echo
	./main example '>' 2018-03-18 | tee greater.out

clean:
	-rm *.o *.so *.a main *.bin *.out

