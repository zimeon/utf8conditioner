# Makefile for utf8conditioner
# Simeon Warner - July 2001...
# [CVS: $Id: Makefile,v 1.3 2003/01/14 19:31:09 simeon Exp $

OBJ = utf8conditioner.o getopt.o
EXECUTABLE = utf8conditioner

CC = gcc
LIBS = 
CFLAGS =

all utf8conditioner: $(OBJ)
	$(CC) $(OBJ) $(LIBS) -o $(EXECUTABLE)

strict:
	glintc utf8conditioner.c getopt.c $(LIBS) -o $(EXECUTABLE)

utf8conditioner.o: utf8conditioner.c getopt.h
	$(CC) $(CFLAGS) -c utf8conditioner.c

getopt.o: getopt.c getopt.h
	$(CC) $(CFLAGS) -c getopt.c

clean:
	rm -f $(OBJ) $(EXECUTABLE)

tar:
	cd ..;tar -zcf /tmp/utf8.tar.gz utf8/utf8conditioner.c utf8/getopt.c utf8/getopt.h utf8/Makefile utf8/testfile; cd utf8
	ls -l /tmp/utf8.tar.gz
