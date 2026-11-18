CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -O2 -pthread

all: mtx

mtx: mtx.c
	$(CC) $(CFLAGS) mtx.c -o mtx

clean:
	rm -f mtx mtx.exe

.PHONY: all clean
