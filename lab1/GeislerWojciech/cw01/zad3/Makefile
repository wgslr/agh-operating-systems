CC=gcc
OPTS=-Wall -Wextra

.PHONY: all clean

all: static dynamic

static: main.c libchararray.a
	$(CC) $^ -o $@ 

shared: main.c libchararray.so
	$(CC) $< -o $@ -L . -l chararray

dynamic: main.c libchararray.so
	$(CC) $< -o $@ -ldl -DDL

libchararray.so: chararray.c
	$(CC) -fPIC -c $^ $(OPTS)
	$(CC) -shared -FPIC -o $@ chararray.o

libchararray.a: chararray.c
	$(CC) -c $^
	ar cr $@ chararray.o

clean:
	-rm *.o *.so *.a 
