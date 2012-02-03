CC=gcc
OBJDIR=./build
ssa2dvd: configuration.o subpictures.o utils.o libass_utils.o  ssa2dvd.o     
	$(CC)   -lm -g -o $(OBJDIR)/ssa2dvd $(OBJDIR)/subpictures.o $(OBJDIR)/ssa2dvd.o $(OBJDIR)/utils.o $(OBJDIR)/libass_utils.o $(OBJDIR)/configuration.o -lgd -lass
	
configuration.o: configuration.c configuration.h
	gcc -c configuration.c -o $(OBJDIR)/configuration.o
	
utils.o: utils.c utils.h
	gcc -c utils.c -o $(OBJDIR)/utils.o
	
libass_utils.o: libass_utils.c libass_utils.h
	gcc -c libass_utils.c -o $(OBJDIR)/libass_utils.o
	
subpictures.o: subpictures.c subpictures.h
	gcc -c subpictures.c -o $(OBJDIR)/subpictures.o
	
ssa2dvd.o: ssa2dvd.c
	gcc -c ssa2dvd.c -o $(OBJDIR)/ssa2dvd.o

clean:
	rm -rf build/*
