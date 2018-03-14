CC=gcc
OPTS=-Wall -Wextra

libchararray.so: chararray.c
	$(CC) -fPIC -c $^ $(OPTS)
	$(CC) -shared -FPIC -o $@ chararray.o

libchararray.a: chararray.c
	$(CC) -c $^
	ar cr $@ chararray.o


.PHONY: clean

clean:
	rm *.o *.so
