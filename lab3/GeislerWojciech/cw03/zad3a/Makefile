CC=gcc
RESULTS=times.txt
O=-O0
MLIM=800
TLIM=10
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
SANITIZER=-fsanitize=address -fsanitize=undefined -fsanitize=leak \
    -lasan -lubsan
.PHONY: test clean debug

main: main.c
	$(CC) $< -o $@ $(CFLAGS) $(SANITIZER)

test: main batch.in
	./main batch.in $(TLIM) $(MLIM)

test-chrome: main chrome.batch
	./$< chrome.batch $(TLIM) $(MLIM) 2>/dev/null

clean:
	-rm *.o *.so *.a main *.bin *.out

