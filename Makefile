.PHONY: build, run, submit, test, clean

CC = emcc
CFLAGS = -std=c++11 -Wall -Wextra -pedantic -O3 -sFETCH

build: index.html

index.html: main.o buffer.o rasterize.o
	${CC} $(CFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) -c $(CFLAGS) $^ -o $@

%.tar:
	tar cf $@ *.cpp *.hpp *.h Makefile implemented.txt

clean:
	rm -rf *.o *.wasm *.js
