all: bin lib inc
bin:
	cd ./src && $(MAKE) && cp relay ../bin
lib:
	cp ./src/*.a ./lib
inc:
	cp ./src/*.h ./inc && cp ./src/Raptor/*.h ./inc

clean:
	-rm -f ./bin/* ./lib/* ./inc/* && cd ./src && make clean

.PHONY: all bin lib inc
