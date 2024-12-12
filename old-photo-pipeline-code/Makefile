CC = gcc
CFLAGS = -I. -g -Wall -Wextra -std=c11 -v -pedantic -D_POSIX_C_SOURCE=200809L
INCLUDES = #
LIBS = -lgd -lpthread -lrt -pthread
VALFLAGS = --tool=callgrind --verbose --log-file=valgrind.log
EXEC = ../old-photo-pipeline

all: $(EXEC)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES)
	@echo Compiled 

$(EXEC): old-photo-pipeline.o Processa_ficheiros.o image-lib.o
	$(CC) -o $(EXEC) old-photo-pipeline.o Processa_ficheiros.o image-lib.o $(LIBS)
	@echo Built

clean:
	rm -fr *.o
	@echo Cleaned

clean-all:
	rm -fr *~ core $(EXEC) *.*o* ../Dataset*/timing* ../Dataset-*/*PAR_B
	@echo Cleaned all

run:
	./$(EXEC) $(ARGS)
	@echo Ran with user input


run_default: 
	./$(EXEC) $(ARGS)
	@echo Ran default
	
run-debug:
	gdb ./$(EXEC) $(ARGS) ./Dataset-1 4 -name

setup_coredumps:
	./setup_coredumps.sh	

coregrind:
	make clean-all
	make all
	valgrind $(VALFLAGS) ./$(EXEC) $(ARGS) ./Dataset-1 4 -name;
	@echo Ran with valgrind