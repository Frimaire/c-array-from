SHELL           = /bin/sh
CFLAGS          = -g -O1 -Wall
CC              = gcc
EXEC            = main.out

.PHONY: clean all test valgrind testhuge

all: clean test.o array-from.o
	$(CC) $(CFLAGS) -o $(EXEC) test.o array-from.o -lm

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $?

clean:
	rm -f *.o $(EXEC)

test:
	echo 0 | ./$(EXEC)
	echo 1 | ./$(EXEC)
	echo 4 | ./$(EXEC)
	echo 62 | ./$(EXEC)
	echo 63 | ./$(EXEC)
	echo 64 | ./$(EXEC)
	echo 65 | ./$(EXEC)
	echo 126 | ./$(EXEC)
	echo 127 | ./$(EXEC)
	echo 128 | ./$(EXEC)
	echo 129 | ./$(EXEC)
	echo 190 | ./$(EXEC)
	echo 191 | ./$(EXEC)
	echo 192 | ./$(EXEC)
	echo 256 | ./$(EXEC)
	echo 1000 | ./$(EXEC)
	echo 1024 | ./$(EXEC)
	echo 1234 | ./$(EXEC)
	echo 1989 | ./$(EXEC)
	echo 2052 | ./$(EXEC)
	echo 3456 | ./$(EXEC)
	echo 5678 | ./$(EXEC)
	echo 10000 | ./$(EXEC)
	echo 23456 | ./$(EXEC)
	echo 54321 | ./$(EXEC)
	echo 87102 | ./$(EXEC)
	echo 87103 | ./$(EXEC)
	echo 87104 | ./$(EXEC)
	echo 87105 | ./$(EXEC)
	echo 100000 | ./$(EXEC)
	echo 200000 | ./$(EXEC)
	echo 500000 | ./$(EXEC)
	echo 1000000 | ./$(EXEC)
	echo 3494142 | ./$(EXEC)
	echo 3494143 | ./$(EXEC)
	echo 3494144 | ./$(EXEC)
	echo 3494145 | ./$(EXEC)
	echo 10000000 | ./$(EXEC)

valgrind:
	echo 10000000 | valgrind ./$(EXEC)

# uses ~8GB memory, be aware!
testhuge:
	echo 1000000000 | ./$(EXEC)

doc:
	pandoc -f mediawiki -t html -s -o README.html README.wiki
	pandoc -f mediawiki -t html -s -o README-zh_CN.html README-zh_CN.wiki
