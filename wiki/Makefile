CC = gcc
CFLAGS = -Wall -O2
INC = -I..
LIBS = ../argon2/argon2.a ../util/util.a ../cgi/cgi.a
OBJS = main.o draw.o format.o data.o

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

wiki.cgi: $(OBJS) $(LIBS)
	$(CC) $(CFLAGS) -o wiki.cgi $(OBJS) $(LIBS)
