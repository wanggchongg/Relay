CC = gcc
INC = ./
LIB = ./

CFLAGS = -I$(INC) -L$(LIB) -g -lpthread -lrelay -Os -Wall
AR = ar rcu
RANLIB = ranlib
RM = -rm -f

RELAY_A = librelay.a
RELAY_O = recv_thread.o send_thread.o decode_thread.o encode_thread.o main.o
RAPTOR_O = Raptor/matrix.o Raptor/raptorcode.o

TARGET = relay

all: raptor $(TARGET)

raptor:
	cd ./Raptor/ && $(MAKE)

$(TARGET): $(RELAY_A)
	$(CC) $(CFLAGS) -o $@

$(RELAY_A): $(RELAY_O)
	$(AR) $@ $(RELAY_O) $(RAPTOR_O)
	$(RANLIB) $@

.c.o:
	$(CC) -g -c -I$(INC) $<
main.o: main.c
recv_thread.o: recv_thread.c
send_thread.o: send_thread.c
decode_thread.o: decode_thread.c
encode_thread.o: encode_thread.c


clean: raptor_clean relay_clean
raptor_clean:
	cd ./Raptor && make clean
relay_clean:
	$(RM) $(RELAY_O)

.PHONY: all raptor .c.o clean raptor_clean relay_clean
