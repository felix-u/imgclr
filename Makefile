CFLAGS=-Wextra -Wall -O3
# LIBS=`pkg-config --cflags`

imgclr: main.c
	$(CC) $(CFLAGS) -o imgclr main.c	

debug: main.c
	$(CC) $(CFLAGS) -g -o imgclr main.c
