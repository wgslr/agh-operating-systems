CC=gcc
RESULTS=times.txt
O=-O0
MLIM=300
TLIM=10
CFLAGS=-lm -Wall -Wextra -Wall \
	-Wno-missing-braces -Wextra -Wno-missing-field-initializers -Wformat=2 \
	-Wswitch-default -Wswitch-enum -Wcast-align -Wpointer-arith \
	-Wbad-function-cast -Wstrict-overflow=5 -Wstrict-prototypes -Winline \
	-Wundef -Wnested-externs -Wcast-qual -Wshadow -Wunreachable-code \
	-Wlogical-op -Wfloat-equal -Wstrict-aliasing=2 -Wredundant-decls \
	-Wold-style-definition \
	#-Werror \
	-fno-omit-frame-pointer -ffloat-store -fno-common -fstrict-aliasing \
	$(O)
.PHONY: test clean

main: main.c
	$(CC) $< -o $@ $(CFLAGS)

long: ../zad3b/long.c
	$(CC) $< -o $@ $(CFLAGS) -lm

big: ../zad3b/big.c
	$(CC) $< -o $@ $(CFLAGS) -lm

test: main long big batch.in
	./main batch.in $(TLIM) $(MLIM)

clean:
	-rm *.o *.so *.a main *.bin *.out

