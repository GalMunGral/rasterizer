.PHONY: build, run, submit, test, clean

CC = g++
CFLAGS = -Wall -Wextra -pedantic -O3

run: program
	./program $(file)

build: program

program: main.cpp lodepng.cpp lodepng.h
	${CC} $(CFLAGS) main.cpp lodepng.cpp -o program

submit: submission.tar

%.tar:
	tar cf $@ -X .gitignore --exclude-vcs .

test: submit
	rm -rf __test__
	mkdir __test__
	tar xf submission.tar -C __test__
	cp $(file) __test__
	cd __test__ && $(MAKE) run file=$(file)

clean:
	rm -rf program *.png *.tar __test__
