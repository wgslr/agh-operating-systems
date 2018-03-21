CC=gcc
RESULTS=times.txt
OLEVEL=2
OPTS=-Wall -Wextra -O$(OLEVEL) -DNDEBUG

.PHONY: test clean

main: main.c
	$(CC) $< -o $@ -A $(OPTS)

test: main test.sh
	./test.sh ./main

clean:
	-rm *.o *.so *.a main *.bin

