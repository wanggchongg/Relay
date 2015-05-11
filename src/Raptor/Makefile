CC = gcc
INC = ./
LIB = ./

CFLAGS = -I$(INC) -g -c -lm -Os -Wall -std=gnu99
AR = ar rcu
RANLIB = ranlib
RM = -rm -f

RAPTOR_A = $(LIB)libraptor.a
RAPTOR_O = matrix.o raptorcode.o

$(RAPTOR_A): $(RAPTOR_O)
	$(AR) $@ $(RAPTOR_O)
	$(RANLIB) $@

.c.o:
	$(CC) $(CFLAGS) $<
matrix.o: matrix.c
raptorcode.o: raptorcode.c

clean:
	$(RM) $(RAPTOR_A) $(RAPTOR_O)

.PHONY: clean .c.o
