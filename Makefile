BUILDFLAGS=-o:speed -vet

all: stb_image imgclr

stb_image: libs/stb_image-v2.27/stb_image.h libs/stb_image_write-v1.16/stb_image_write.h
	cd libs/stb_image-v2.27 && \
		$(CC) -c -O3 -Os -fPIC stb_image.c && \
		$(AR) rcs ../stb_image.a stb_image.o && \
		rm *.o
	cd libs/stb_image_write-v1.16 && \
		$(CC) -c -O3 -Os -fPIC stb_image_write.c && \
		$(AR) rcs ../stb_image_write.a stb_image_write.o && \
		rm *.o

imgclr: src/main.odin
	odin build src -out:./imgclr $(BUILDFLAGS)
