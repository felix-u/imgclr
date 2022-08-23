BUILDFLAGS=-Wextra -Wall
RELEASEFLAGS=-O3

imgclr: src/main.c
	$(CC) $(BUILDFLAGS) -o imgclr src/main.c

release: src/main.c
	$(CC) $(BUILDFLAGS) $(RELEASEFLAGS) -o imgclr src/main.c
