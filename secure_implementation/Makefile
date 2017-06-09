.SILENT:

CC = gcc
CFLAGS = -lmraa -lssl -lcrypto -g
AUX = Makefile README main.h main.c
TARBALL_NAME = group3.tar.gz

default:
	@$(CC) $(CFLAGS) -o main main.h main.c

clean:
	@rm -f main *.tar.gz

dist: 
	@tar -czf $(TARBALL_NAME) $(AUX)
