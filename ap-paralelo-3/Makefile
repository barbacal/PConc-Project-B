# Define required macros here
# SHELL = /bin/bash

CC = gcc
CFLAGS = -I. -g -Wall -std=c11 -v -pedantic  
INCLUDES =#
LIBS = -lgd

all: ap-paralelo-3

%.o: %.c ${DEPS}
	${CC} -c -o $@ $< $(CFLAGS) ${INCLUDES}
	@echo Compiled 

ap-paralelo-3: ap-paralelo-3.o Processa_ficheiros.o image-lib.o
	${CC} -o ap-paralelo-3 ap-paralelo-3.o Processa_ficheiros.o image-lib.o ${LIBS}


clean:
	rm -f *.o *~ core ap-paralelo-3
	@echo Cleanned
   
clean-data:
	rm -f *.out*
	@echo Cleanned data
