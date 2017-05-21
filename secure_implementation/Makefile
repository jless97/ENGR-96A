.SILENT:

CC = gcc
CFLAGS = -lmraa -g
AUX = Makefile README main.c
TARBALL_NAME = group3.tar.gz

default:
	@$(CC) $(CFLAGS) -o main main.c

clean:
	@rm -f main *.tar.gz 

dist: 
	@tar -czf $(TARBALL_NAME) $(AUX)