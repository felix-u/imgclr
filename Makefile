CFLAGS=-Wextra -Wall -O3
DEBUGFLAGS=-g

imgclr: src/main.c src/args.c
	$(CC) $(CFLAGS) -o imgclr src/main.c

debug: src/
	$(CC) $(CFLAGS) $(DEBUGFLAGS) -o imgclr src/main.c
