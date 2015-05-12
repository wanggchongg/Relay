BIN = ./bin/
INC = ./inc/
LIB = ./lib/
SRC = ./src/

RELAY_E = ./src/relay
RELAY_A = ./src/*.a
RELAY_H = ./src/*.h ./src/Raptor/*.h

exe:
	cd $(SRC) && $(MAKE)

install: $(RELAY_E)
	@if [ -d $(BIN) ] && [ -d $(LIB) ] && [ -d $(INC) ]; \
		then \
		cp $(RELAY_E) $(BIN); \
		cp $(RELAY_A) $(LIB); \
		cp $(RELAY_H) $(INC); \
		echo " *.exe Installed in $(BIN) "; \
		echo " *.a Installed in $(LIB) "; \
		echo " *.h Installed in $(INC) "; \
	else \
		echo "Sorry, $(BIN) or $(LIB) or $(INC) does not exist"; \
	fi

clean:
	cd $(SRC) && make clean

.PHONY: exe install bin lib inc clean
