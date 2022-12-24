CC = emcc
CFLAGS = -std=c++11 -Wall -Wextra -pedantic -O3 -sFETCH -sASSERTIONS -sINITIAL_MEMORY=134217728

build: index.html

index.html: main.o buffer.o rasterize.o
	mkdir -p docs
	${CC} $(CFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) -c $(CFLAGS) $^ -o $@

clean:
	rm -rf *.o index.*
