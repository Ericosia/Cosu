CC=gcc
CFLAGS=-Wall -Wextra -ggdb


main: main.c
	$(CC) $(CFLAGS) -o main env.c main.c -lcurl
