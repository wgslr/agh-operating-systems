CC=gcc
RESULTS=results3a.txt
OLEVEL=0
OPTS=-Wall -Wextra -O$(OLEVEL)

.PHONY: all clean test test_dynamic test_shared test_static test_all

all: static shared dynamic 

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
	$(AR) cr $@ chararray.o

test_all: 
	$(MAKE) test_static
	$(MAKE) test_shared
	$(MAKE) test_dynamic

test:
	echo Library: $(EXEC) >> $(RESULTS)
	echo O-level: $(OLEVEL) >> $(RESULTS)
	./$(EXEC) 1000 100 0 create_table >> $(RESULTS)
	./$(EXEC) 1000 100 1 create_table >> $(RESULTS)
	./$(EXEC) 1000 100 0 add 1000 remove 1000 remove_and_add 1000 >> $(RESULTS)
	./$(EXEC) 1000 100 1 add 1000 remove 1000 remove_and_add 1000 >> $(RESULTS)
	./$(EXEC) 1000 100 0 search_element 500 >> $(RESULTS)
	./$(EXEC) 1000 100 1 search_element 500 >> $(RESULTS)

test_dynamic: EXEC=dynamic
test_dynamic: dynamic test

test_shared: EXEC=shared
test_shared: shared test

test_static: EXEC=static
test_static: static test

clean:
	-rm *.o *.so *.a static dynamic shared
