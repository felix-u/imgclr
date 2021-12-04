CFLAGS=-Wextra -Wall -O3
LIBS=`pkg-config --cflags --libs MagickWand`

mclr: main.c
	$(CC) $(CFLAGS) -o mclr main.c $(LIBS)
