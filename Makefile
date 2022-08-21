BUILDFLAGS=-o:speed -vet

imgclr: src/main.odin
	odin build src -out:./imgclr $(BUILDFLAGS)
