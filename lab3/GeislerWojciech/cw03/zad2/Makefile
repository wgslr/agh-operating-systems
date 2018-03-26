CC=gcc
RESULTS=times.txt
O=-O0
CFLAGS=-Wall -Wextra -Wall \
	-Wno-missing-braces -Wextra -Wno-missing-field-initializers -Wformat=2 \
	-Wswitch-default -Wswitch-enum -Wcast-align -Wpointer-arith \
	-Wbad-function-cast -Wstrict-overflow=5 -Wstrict-prototypes -Winline \
	-Wundef -Wnested-externs -Wcast-qual -Wshadow -Wunreachable-code \
	-Wlogical-op -Wfloat-equal -Wstrict-aliasing=2 -Wredundant-decls \
	-Wold-style-definition \
	-Werror \
	-fno-omit-frame-pointer -ffloat-store -fno-common -fstrict-aliasing \
	$(O)
#SANITIZER=-fsanitize=address -fsanitize=undefined -fsanitize=leak \
    -lasan -lubsan
.PHONY: test clean

main: main.c
	$(CC) $< -o $@ $(CFLAGS) $(SANITIZER)

test: main batch.in
	./$< batch.in

clean:
	-rm *.o *.so *.a main *.bin *.out

