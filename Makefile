#################################################### 
##                                                ## 
##          Makefile for Project Part A           ## 
##                   Apr 4, 2017                  ## 
##                                                ## 
####################################################  

# CC      = gcc
# CFLAGS  =-std=c99 -Wall -Wextra -O0 -ggdb

main:
	##########################################################################
	## compile together with project Part B, not yet came out.              ##
	## If you want to compile with test .c file with Part A, run: make test ##
	##########################################################################

test:
	make -f ./TestMakefile

clean:
	rm -rf *.o *.dSYM

reset:
	make clean
	rm test
	rm -r ./data
   

