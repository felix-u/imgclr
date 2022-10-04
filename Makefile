CFLAGS=-std=c99 -pedantic -Wshadow -Wstrict-aliasing -Wstrict-overflow \
	   -Wextra -Wall -Wno-unused-but-set-variable
LIBS=-lm
DEBUGFLAGS=-Og -g
RELEASEFLAGS=-O3 -s

imgclr: src/main.c src/args.c src/colour.c src/dither.c
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(LIBS) -o imgclr src/main.c

release: src/
	$(CC) $(CFLAGS) $(RELEASEFLAGS) $(LIBS) -o imgclr src/main.c
