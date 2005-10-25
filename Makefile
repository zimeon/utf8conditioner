# Makefile for utf8conditioner
# Simeon Warner - July 2001...
# [CVS: $Id: Makefile,v 1.12 2005/10/25 23:27:09 simeon Exp $

OBJ = utf8conditioner.o getopt.o
EXECUTABLE = utf8conditioner
PACKAGE = utf8/utf8conditioner.c utf8/getopt.c utf8/getopt.h utf8/Makefile utf8/COPYING utf8/README utf8/HISTORY utf8/test
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

.PHONY: clean
clean:
	rm -f $(OBJ) $(EXECUTABLE)

.PHONY: tar
tar:
	cd ..;tar -zcf /tmp/utf8conditioner.tar.gz $(PACKAGE); cd utf8
	ls -l /tmp/utf8conditioner.tar.gz

.PHONY: zip
zip:
	cd ..;zip -r /tmp/utf8conditioner.zip $(PACKAGE); cd utf8  
	ls -l /tmp/utf8conditioner.zip

.PHONY: test
test:
	@echo -n "test[01] - option -c ..................... "
	@cat test/testfile | ./$(EXECUTABLE) -c 2> $(TEST_TMP)
	@r=`diff -I 'Id' $(TEST_TMP) test/test-result-c.txt 2>&1`
	@if [ -n "$r" ]; then echo "FAILED"; else echo "PASS"; fi
	@echo -n "test[02] - options -c -x.................. "
	@cat test/testfile | ./$(EXECUTABLE) -c -x 2> $(TEST_TMP)
	@r=`diff -I 'Id' $(TEST_TMP) test/test-result-cx.txt 2>&1`
	@if [ -n "$r" ]; then echo "FAILED"; else echo "PASS"; fi
	@echo -n "test[03] - options -c -X 1.1.............. "
	@cat test/testfile | ./$(EXECUTABLE) -c -X1.1 2> $(TEST_TMP)
	@r=`diff -I 'Id' $(TEST_TMP) test/test-result-cX1.1.txt 2>&1`
	@if [ -n "$r" ]; then echo "FAIL"; else echo "PASS"; fi
	@echo -n "test[04] - UTF-8-test-1 (good) ........... "
	@cat test/UTF-8-test-1.txt | ./$(EXECUTABLE) -c 2> $(TEST_TMP)
	@if [ -S $(TEST_TMP) ]; then echo "FAIL"; else echo "PASS"; fi
	@echo -n "test[05] - UTF-8-test-2 (good) ........... "
	@cat test/UTF-8-test-2.txt | ./$(EXECUTABLE) -c 2> $(TEST_TMP)
	@if [ -S $(TEST_TMP) ]; then echo "FAIL"; else echo "PASS"; fi
	@echo -n "test[06] - UTF-8-test-2-disallowed (bad) . "
	@cat test/UTF-8-test-2-disallowed.txt | ./$(EXECUTABLE) -c 2> $(TEST_TMP)
	@r=`diff -I 'Id' $(TEST_TMP) test/test-result-2-disallowed.txt 2>&1`
	@if [ -n "$r" ]; then echo "FAIL"; else echo "PASS"; fi
	@echo -n "test[07] - UTF-8-test-3 (bad) ............ "
	@cat test/UTF-8-test-3.txt | ./$(EXECUTABLE) -c 2> $(TEST_TMP)
	@r=`diff -I 'Id' $(TEST_TMP) test/test-result-3.txt 2>&1`
	@if [ -n "$r" ]; then echo "FAIL"; else echo "PASS"; fi
	@echo -n "test[08] - UTF-8-test-4 (bad) ............ "
	@cat test/UTF-8-test-4.txt | ./$(EXECUTABLE) -c 2> $(TEST_TMP)
	@r=`diff -I 'Id' $(TEST_TMP) test/test-result-4.txt 2>&1`
	@if [ -n "$r" ]; then echo "FAIL"; else echo "PASS"; fi
	@echo -n "test[09] - UTF-8-test-5 (bad) ............ "
	@cat test/UTF-8-test-5.txt | ./$(EXECUTABLE) -c 2> $(TEST_TMP)
	@r=`diff -I 'Id' $(TEST_TMP) test/test-result-5.txt 2>&1`
	@if [ -n "$r" ]; then echo "FAIL"; else echo "PASS"; fi
	@echo -n "test[10] - entities-good (good) .......... "
	@cat test/entities-good.txt | ./$(EXECUTABLE) -c -x 2> $(TEST_TMP)
	@if [ -S "$(TEST_TMP)" ]; then echo "FAIL"; else echo "PASS"; fi
	@echo -n "test[11] - entities-bad not XML (good) ... "
	@cat test/entities-bad.txt | ./$(EXECUTABLE) -c 2> $(TEST_TMP)
	@if [ -S "$(TEST_TMP)" ]; then echo "FAIL"; else echo "PASS"; fi
	@echo -n "test[12] - entities-bad -x (bad) ......... "
	@cat test/entities-bad.txt | ./$(EXECUTABLE) -c -x 2> $(TEST_TMP)
	@r=`diff -I 'Id' $(TEST_TMP) test/test-result-entities-bad-x.txt 2>&1`
	@if [ -n "$r" ]; then echo "FAIL"; else echo "PASS"; fi
	@rm -f $(TEST_TMP)
