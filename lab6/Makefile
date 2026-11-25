CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11

all: threads writer reader

threads: threads.c
	$(CC) $(CFLAGS) -pthread threads.c -o threads

writer: writer.c
	$(CC) $(CFLAGS) writer.c -o writer

reader: reader.c
	$(CC) $(CFLAGS) reader.c -o reader

clean:
	rm -f threads writer reader

.PHONY: all clean
