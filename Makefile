.PHONY: build, run, submit, test, clean

CC = g++
CFLAGS = -std=c++11 -Wall -Wextra -pedantic -O3

debug: CFLAGS = -std=c++11 -Wall -Wextra -pedantic -O0 -g
debug: program

run: program
	./program $(file)

build: program

program: main.o lodepng.o buffer.o rasterize.o
	${CC} $(CFLAGS) $^ -o $@

submit: submission.tar

%.o: %.cpp
	$(CC) -c $(CFLAGS) $^ -o $@

%.tar:
	tar cf $@ *.cpp *.hpp *.h Makefile 

test: submit
	rm -rf __test__
	mkdir __test__
	tar xf submission.tar -C __test__
	cp $(file) __test__
	cd __test__ && $(MAKE) run file=$(file)

clean:
	rm -rf *.o program *.tar *.png *.txt __test__ 
