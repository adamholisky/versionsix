CC = gcc

all: afs

run: all 
	./afs

afs: afs.c
	$(CC) -O0 $< -g -o afs

clean:
	rm afs