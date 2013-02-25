CC = gcc
CFLAGS = -Wall -O2
INC = -I..

all: cgi.a

clean:
	rm -f cgi.a
	rm -f *.o

input.o: input.c
	$(CC) $(CFLAGS) -c input.c $(INC)
output.o: output.c
	$(CC) $(CFLAGS) -c output.c $(INC)

cgi.a: output.o input.o
	ar r cgi.a output.o input.o
