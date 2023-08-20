CC = emcc
CFLAGS = -std=c++11 -Wall -Wextra -pedantic -O3 -sFETCH -sUSE_SDL -sASSERTIONS -sINITIAL_MEMORY=134217728

build: index.html

index.html: main.o buffer.o rasterize.o shell.html
	mkdir -p docs
	${CC} $(CFLAGS) $(filter-out %.html, $^) -o $@ --shell-file shell.html

shell.html: ;

%.o: %.cpp
	$(CC) -c $(CFLAGS) $^ -o $@

clean:
	rm -rf *.o index.*
