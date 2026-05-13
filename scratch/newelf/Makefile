CC = gcc

all: libtest

run: all 
	./libtest

libtest:
	$(CC) -Wno-error -O0 list.c -g -c
	$(CC) -Wno-error -O0 elf.c -g -c
	$(CC) -Wno-error -O0 lib.c -g -c
	$(CC) -Wno-error -O0 list.o elf.o lib.o -g -o libtest

clean:
	rm libtest
	rm *.o
