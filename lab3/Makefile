CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11

SOURCES = $(wildcard *.c)
PROGRAMS = $(SOURCES:.c=)

all: $(PROGRAMS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(PROGRAMS)

.PHONY: all clean
