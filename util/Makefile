CC = gcc
CFLAGS = -Wall -O2
INC =

all: util.a

clean:
	rm -f util.a
	rm -f *.o

md5.o: md5.c
	$(CC) $(CFLAGS) -c md5.c $(INC)

binary.o: binary.c
	$(CC) $(CFLAGS) -c binary.c $(INC)

util.a: binary.o md5.o
	ar r util.a binary.o md5.o
