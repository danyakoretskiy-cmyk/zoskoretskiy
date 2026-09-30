CFLAGS = -Wall -Wextra -std=gnu11

all: mycat mygrep myls mychmod

mycat: mycat.c
	cc $(CFLAGS) mycat.c -o mycat

mygrep: mygrep.c
	cc $(CFLAGS) mygrep.c -o mygrep

myls: myls.c
	cc $(CFLAGS) myls.c -o myls

mychmod: mychmod.c
	cc $(CFLAGS) mychmod.c -o mychmod

clean:
	rm -f mycat mygrep myls mychmod

.PHONY: all clean
