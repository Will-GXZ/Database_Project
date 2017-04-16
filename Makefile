#################################################### 
##                                                ## 
##          Makefile for Project Part A           ## 
##                   Apr 4, 2017                  ## 
##                                                ## 
####################################################  

# CC      = gcc
# CFLAGS  =-std=c99 -Wall -Wextra -O0 -ggdb

# compile together with project Part B, not yet came out.



test:
	make -f ./TestMakefile

clean:
	rm -rf *.o *.dSYM

reset:
	make clean
	rm test
	rm -r ./data
   

