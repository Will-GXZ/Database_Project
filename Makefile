####################################################
##                                                ##
##          Makefile for Project Part A           ##
##                  Apr 4, 2017                   ##       
##                                                ##
####################################################
CC		= gcc
CFLAGS	=-std=c99 -Wall -Wextra -O0 -ggdb

test:		projectA.o my_test.o
	@echo '****************  Using Makefile to compile ****************'

	@echo '*******  To delete *.o, run make clean, to delete all \
	run make reset *******'
	
	@echo ' '

	${CC} ${CFLAGS} -o test projectA.o my_test.o

projectA.o:	projectA.c projectA.h
my_test.o: my_test.c projectA.h

clean:
	rm -rf *.o *.dSYM

reset:
	make clean
	rm test

