CC=gcc
CFLAGS=-Wall -Wextra -ggdb


test: test.c
	$(CC) $(CFLAGS) -o test env.c test.c cosu.c -lcurl
