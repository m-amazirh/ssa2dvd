CC=gcc
OBJDIR=./build
ssa2dvd: subpictures.c ssa2dvd.c
	mkdir -p $(OBJDIR)
	$(CC)  -DNDEBUG -g -o build/ssa2dvd subpictures.c ssa2dvd.c -lgd -lass
clean:
	rm -rf build/*
