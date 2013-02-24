CC = gcc
CFLAGS = -Wall -O2
INC = -I..
LIBCGI = ../cgi/libcgi.a

all: wiki.cgi

clean:
	rm -f wiki.cgi
	rm -f *.o

data.o: data.c
	$(CC) $(CFLAGS) -c $(INC) data.c
format.o: format.c
	$(CC) $(CFLAGS) -c $(INC) format.c
draw.o: draw.c
	$(CC) $(CFLAGS) -c $(INC) draw.c
main.o: main.c
	$(CC) $(CFLAGS) -c $(INC) main.c

OBJ = main.o draw.o format.o data.o
wiki.cgi: $(OBJ) $(LIBCGI)
	$(CC) $(CFLAGS) -o wiki.cgi $(OBJ) $(LIBCGI)
