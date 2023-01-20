VERSION=0.1-dev

CFLAGS=-std=c99 -pedantic -Wshadow -Wstrict-aliasing -Wstrict-overflow \
	   -Wextra -Wall -Wno-unused-but-set-variable
DEBUGFLAGS=-Og -g
RELEASEFLAGS=-O3 -s
LIBS=-lm

imgclr: src/*
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(LIBS) -o imgclr src/main.c

release: src/
	$(CC) $(CFLAGS) $(RELEASEFLAGS) $(LIBS) -o imgclr src/main.c -march=native

cross: src/*
	mkdir -p release
	zig cc -static -target x86_64-windows     $(CFLAGS) $(RELEASEFLAGS) $(LIBS) -o release/imgclr-v$(VERSION)-x86_64-win.exe  src/main.c
	zig cc -static -target aarch64-windows    $(CFLAGS) $(RELEASEFLAGS) $(LIBS) -o release/imgclr-v$(VERSION)-aarch64-win.exe src/main.c
	zig cc -static -target x86_64-linux-musl  $(CFLAGS) $(RELEASEFLAGS) $(LIBS) -o release/imgclr-v$(VERSION)-x86_64-linux    src/main.c
	zig cc -static -target aarch64-linux-musl $(CFLAGS) $(RELEASEFLAGS) $(LIBS) -o release/imgclr-v$(VERSION)-aarch64-linux   src/main.c
	zig cc -static -target x86_64-macos       $(CFLAGS) $(RELEASEFLAGS) $(LIBS) -o release/imgclr-v$(VERSION)-x86_64-macos    src/main.c
	zig cc -static -target aarch64-macos      $(CFLAGS) $(RELEASEFLAGS) $(LIBS) -o release/imgclr-v$(VERSION)-aarch64-macos   src/main.c

copy:
	cp imgclr ~/.local/bin/

install: release copy
