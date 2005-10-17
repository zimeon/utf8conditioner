# Makefile for utf8conditioner
# Simeon Warner - July 2001...
# [CVS: $Id: Makefile,v 1.7 2005/10/17 20:45:02 simeon Exp $

OBJ = utf8conditioner.o getopt.o
EXECUTABLE = utf8conditioner
PACKAGE = utf8/utf8conditioner.c utf8/getopt.c utf8/getopt.h utf8/Makefile utf8/testfile utf8/COPYING utf8/README utf8/HISTORY
TEST_TMP = /tmp/utf8conditioner_test

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
	cd ..;tar -zcf /tmp/utf8conditioner.tar.gz $(PACKAGE); cd utf8
	ls -l /tmp/utf8conditioner.tar.gz

zip:
	cd ..;zip /tmp/utf8conditioner.zip $(PACKAGE); cd utf8  
	ls -l /tmp/utf8conditioner.zip

test:
	@echo -n "test[01] - option -c ..................... "
	@cat testfile | ./$(EXECUTABLE) -c 2> $(TEST_TMP)
	@set r=`diff -I 'Id:' $(TEST_TMP) test-result-c.txt`
	@if [ -n "$r" ]; then echo "FAILED"; else echo "PASS"; fi
	@echo -n "test[02] - options -c -x.................. "
	@cat testfile | ./$(EXECUTABLE) -c -x 2> $(TEST_TMP)
	@set r=`diff -I 'Id:' $(TEST_TMP) test-result-cx.txt`
	@if [ -n "$r" ]; then echo "FAILED"; else echo "PASS"; fi
	@echo -n "test[03] - options -c -X 1.1.............. "
	@cat testfile | ./$(EXECUTABLE) -c -X1.1 2> $(TEST_TMP)
	@set r=`diff -I 'Id:' $(TEST_TMP) test-result-cX1.1.txt`
	@if [ -n "$r" ]; then echo "FAIL"; else echo "PASS"; fi
	@echo -n "test[04] - UTF-8-test-1 (good) ........... "
	@cat UTF-8-test-1.txt | ./$(EXECUTABLE) -c 2> $(TEST_TMP)
	@if [ -S $(TEST_TMP) ]; then echo "FAIL"; else echo "PASS"; fi
	@echo -n "test[05] - UTF-8-test-2 (good) ........... "
	@cat UTF-8-test-2.txt | ./$(EXECUTABLE) -c 2> $(TEST_TMP)
	@if [ -S $(TEST_TMP) ]; then echo "FAIL"; else echo "PASS"; fi
	@echo -n "test[06] - UTF-8-test-2-disallowed (bad) . "
	@cat UTF-8-test-2-disallowed.txt | ./$(EXECUTABLE) -c 2> $(TEST_TMP)
	@set r=`diff -I 'Id:' $(TEST_TMP) test-result-2-disallowed.txt 2>&1`
	@if [ -n "$r" ]; then echo "FAIL"; else echo "PASS"; fi
	@echo -n "test[07] - UTF-8-test-3 (bad) ............ "
	@cat UTF-8-test-3.txt | ./$(EXECUTABLE) -c 2> $(TEST_TMP)
	@set r=`diff -I 'Id:' $(TEST_TMP) test-result-3.txt`
	@if [ -n "$r" ]; then echo "FAIL"; else echo "PASS"; fi
	@echo -n "test[08] - UTF-8-test-4 (bad) ............ "
	@cat UTF-8-test-4.txt | ./$(EXECUTABLE) -c 2> $(TEST_TMP)
	@set r=`diff -I 'Id:' $(TEST_TMP) test-result-4.txt`
	@if [ -n "$r" ]; then echo "FAIL"; else echo "PASS"; fi
	@echo -n "test[09] - UTF-8-test-5 (bad) ............ "
	@cat UTF-8-test-5.txt | ./$(EXECUTABLE) -c 2> $(TEST_TMP)
	@set r=`diff -I 'Id:' $(TEST_TMP) test-result-5.txt`
	@if [ -n "$r" ]; then echo "FAIL"; else echo "PASS"; fi
	@rm -f $(TEST_TMP)
