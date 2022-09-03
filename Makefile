.PHONY: build, run, submit, test, clean

CC = g++
CFLAGS = -Wall -Wextra -pedantic -O3

run: program
	./program $(file)

build: program

program: main.o lodepng.o image.o
	${CC} $(CFLAGS) $^ -o $@

submit: submission.tar

%.o: %.cpp
	$(CC) -c $(CFLAGS) $^ -o $@

%.tar:
	tar cf $@ -X .gitignore --exclude-vcs .

test: submit
	rm -rf __test__
	mkdir __test__
	tar xf submission.tar -C __test__
	cp $(file) __test__
	cd __test__ && $(MAKE) run file=$(file)

clean:
	rm -rf *.o program *.png *.tar __test__
