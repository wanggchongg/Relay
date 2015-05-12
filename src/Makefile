CC = gcc
INC = ./
LIB = ./

LFLAGS = -I$(INC)  -L$(LIB) -g -Os -Wall -std=gnu99 -lrelay -lpthread -lm
CFLAGS = -I$(INC) -g -c -Os -Wall -std=gnu99
AR = ar rcu
RANLIB = ranlib
RM = -rm -f

RELAY_A = $(LIB)librelay.a
RELAY_O =  recv_thread.o send_thread.o decode_thread.o encode_thread.o
RAPTOR_O = Raptor/matrix.o Raptor/raptorcode.o

TARGET = relay

all: raptor $(TARGET)

raptor:
	cd ./Raptor/ && $(MAKE)

$(TARGET): $(RELAY_A) main.o
	$(CC) main.o -o $@ $(LFLAGS)

$(RELAY_A): $(RELAY_O)
	$(AR) $@ $(RELAY_O) $(RAPTOR_O)
	$(RANLIB) $@

.c.o:
	$(CC) $(CFLAGS) $<
main.o: main.c
recv_thread.o: recv_thread.c
send_thread.o: send_thread.c
decode_thread.o: decode_thread.c
encode_thread.o: encode_thread.c


clean: raptor_clean relay_clean
raptor_clean:
	cd ./Raptor && make clean
relay_clean:
	$(RM) $(RELAY_O) $(RELAY_A) $(TARGET)

.PHONY: all raptor .c.o clean raptor_clean relay_clean
