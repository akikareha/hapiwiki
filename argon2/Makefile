CC = gcc
CFLAGS = -Wall -O2
INC = 

all: argon2.a

clean:
	rm -f argon2.a
	rm -f *.o

argon2.o: argon2.c
	$(CC) $(CFLAGS) -c argon2.c $(INC)

blake2b.o: blake2b.c
	$(CC) $(CFLAGS) -c blake2b.c $(INC)

core.o: core.c
	$(CC) $(CFLAGS) -c core.c $(INC)

encoding.o: encoding.c
	$(CC) $(CFLAGS) -c encoding.c $(INC)

ref.o: ref.c
	$(CC) $(CFLAGS) -c ref.c $(INC)

thread.o: thread.c
	$(CC) $(CFLAGS) -c thread.c $(INC)

argon2.a: argon2.o blake2b.o core.o encoding.o ref.o thread.o
	ar r argon2.a argon2.o blake2b.o core.o encoding.o ref.o thread.o
