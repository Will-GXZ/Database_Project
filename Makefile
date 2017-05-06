#################################################### 
##                                                ## 
##          Makefile for Project Part B           ## 
##                   May 3, 2017                  ## 
##                                                ## 
####################################################  

CC      = gcc
CFLAGS  =-std=gnu11 -Wall -Wextra -O0 -ggdb

testB:	projectA.o projectB.o my_testB.o 
	@echo '________________________________________________________________'
	@echo '|                                                               |'
	@echo '|                  Using Makefile to compile                    |'
	@echo '|                                                               |'
	@echo '|                  Compiling using my_testB.c                   |'
	@echo '|                                                               |'
	@echo '|               --To delete *.o, run make clean                 |'
	@echo '|               --To delete all, run make reset                 |'
	@echo '|_______________________________________________________________|'
	
	${CC} ${CFLAGS} -o testB projectA.o projectB.o my_testB.o

projectB.o: projectA.h projectB.c projectB.h record.h 
my_testB.o: my_testB.c projectA.h projectB.h
projectA.o: projectA.c projectA.h


testA:
	make -f ./TestMakefileA # compile using Part A Makefile

clean:
	rm -rf *.o *.dSYM

reset:
	make clean
	rm -rf ./data testA testB

   

