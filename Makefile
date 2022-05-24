CFLAGS=-Wextra -Wall -O3
# LIBS=`pkg-config --cflags`

mclr: main.c
	$(CC) $(CFLAGS) -o mclr main.c

debug: main.c
	$(CC) $(CFLAGS) -g -o mclr main.c
