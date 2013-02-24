CC = gcc
CFLAGS = -Wall -O2
INC = -I..
LIBCGI = ../cgi/libcgi.a

all: hello.cgi date.cgi env.cgi post.cgi post_compound.cgi upload.cgi

clean:
	rm -f hello.cgi date.cgi env.cgi post.cgi post_compound.cgi upload.cgi

hello.cgi: hello.c
	$(CC) $(CFLAGS) -o hello.cgi hello.c

date.cgi: date.c
	$(CC) $(CFLAGS) -o date.cgi date.c

env.cgi: env.c
	$(CC) $(CFLAGS) -o env.cgi env.c

post.cgi: post.c
	$(CC) $(CFLAGS) -o post.cgi $(INC) post.c $(LIBCGI)

post_compound.cgi: post.c
	$(CC) $(CFLAGS) -o post_compound.cgi -DCOMPOUND $(INC) post.c $(LIBCGI)

upload.cgi: upload.c
	$(CC) $(CFLAGS) -o upload.cgi $(INC) upload.c $(LIBCGI)
