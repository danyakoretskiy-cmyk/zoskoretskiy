CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -pthread

all: mtx

mtx: mtx.c
	$(CC) $(CFLAGS) mtx.c -o mtx

clean:
	rm -f mtx

.PHONY: all clean
