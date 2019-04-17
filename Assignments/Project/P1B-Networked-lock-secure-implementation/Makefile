.SILENT:

CC = gcc
CFLAGS = -lmraa -lssl -lcrypto -g
AUX = Makefile README.md iot_secure_door_unlock.h iot_secure_door_unlock.c
TARBALL_NAME = group3.tar.gz

default:
	@$(CC) $(CFLAGS) -o iot_secure_door_unlock iot_secure_door_unlock.h iot_secure_door_unlock.c

clean:
	@rm -f iot_secure_door_unlock *.tar.gz

dist: 
	@tar -czf $(TARBALL_NAME) $(AUX)

