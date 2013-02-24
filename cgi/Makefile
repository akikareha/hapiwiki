CC = gcc
CFLAGS = -Wall -O2
INC = -I..

all: libcgi.a

clean:
	rm -f libcgi.a
	rm -f *.o

input.o: input.c
	$(CC) $(CFLAGS) -c input.c $(INC)
output.o: output.c
	$(CC) $(CFLAGS) -c output.c $(INC)

libcgi.a: output.o input.o
	ar r libcgi.a output.o input.o
