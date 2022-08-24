CFLAGS=-Wextra -Wall -O3

imgclr: src/main.c
	$(CC) $(CFLAGS) -o imgclr src/main.c
