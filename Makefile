# Makefile for utf8conditioner
# Simeon Warner - July 2001...
# [CVS: $Id: Makefile,v 1.1 2001/08/01 21:01:17 simeon Exp $

OBJ = utf8conditioner.o getopt.o
EXECUTABLE = utf8conditioner

LIBS = 
CFLAGS =
#CC = g++
CC = glintc

.SUFFIXES:
.SUFFIXES: .C .o

.C.o:
	$(CC) -c $*.C $(CFLAGS)

all utf8conditioner: $(OBJ)
	$(CC) $(OBJ) $(LIBS) -o $(EXECUTABLE)

utf8conditioner.o: utf8conditioner.c getopt.h
	$(CC) -c utf8conditioner.c

getopt.o: getopt.c getopt.h
	$(CC) -c getopt.c
