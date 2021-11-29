CFLAGS=-Wextra -Wall
LIBS=`pkg-config --cflags --libs MagickWand`

mclr: main.c
	$(CC) $(CFLAGS) -o mclr main.c $(LIBS)
