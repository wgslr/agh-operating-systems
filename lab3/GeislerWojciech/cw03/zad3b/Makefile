CC=gcc
RESULTS=times.txt
O=-O0
MLIM=200
TLIM=5
CFLAGS=-lm -Wall -Wextra -Wall \
	-Wno-missing-braces -Wextra -Wno-missing-field-initializers -Wformat=2 \
	-Wswitch-default -Wswitch-enum -Wcast-align -Wpointer-arith \
	-Wbad-function-cast -Wstrict-overflow=5 -Wstrict-prototypes -Winline \
	-Wundef -Wnested-externs -Wcast-qual -Wshadow -Wunreachable-code \
	-Wlogical-op -Wfloat-equal -Wstrict-aliasing=2 -Wredundant-decls \
	-Wold-style-definition \
	-Werror \
	-fno-omit-frame-pointer -ffloat-store -fno-common -fstrict-aliasing \
	$(O)
SANITIZER=
.PHONY: test clean debug

main: main.c
	$(CC) $< -o $@ $(CFLAGS) $(SANITIZER)

long: long.c
	$(CC) $< -o $@ $(CFLAGS) $(SANITIZER)

big: big.c
	$(CC) $< -o $@ $(CFLAGS) $(SANITIZER)

test: test-big test-long

test-big: main big
	./$< big.batch $(TLIM) $(MLIM)

test-long: main long
	./$< long.batch $(TLIM) $(MLIM)

clean:
	-rm *.o *.so *.a *.bin *.out main big long

