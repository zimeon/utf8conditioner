# Makefile for utf8conditioner
# Simeon Warner - July 2001...
# [CVS: $Id: Makefile,v 1.2 2001/08/01 22:03:12 simeon Exp $

OBJ = utf8conditioner.o getopt.o
EXECUTABLE = utf8conditioner

LIBS = 
CFLAGS =
CC = gcc

all utf8conditioner: $(OBJ)
	$(CC) $(OBJ) $(LIBS) -o $(EXECUTABLE)

strict:
	glintc utf8conditioner.c getopt.c $(LIBS) -o $(EXECUTABLE)

utf8conditioner.o: utf8conditioner.c getopt.h
	$(CC) -c utf8conditioner.c

getopt.o: getopt.c getopt.h
	$(CC) -c getopt.c

tar:
	cd ..;tar -zcf /tmp/utf8.tar.gz utf8/utf8conditioner.c utf8/getopt.c utf8/getopt.h utf8/Makefile utf8/testfile; cd utf8
	ls -l /tmp/utf8.tar.gz
