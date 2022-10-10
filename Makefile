CC = emcc
CFLAGS = -std=c++11 -Wall -Wextra -pedantic -O3 -sFETCH -sASSERTIONS -sINITIAL_MEMORY=111017984

build: docs/index.html

docs/index.html: main.o buffer.o rasterize.o
	mkdir -p docs
	${CC} $(CFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) -c $(CFLAGS) $^ -o $@

clean:
	rm -rf *.o docs
