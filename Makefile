
test:
	gcc -g -Wall -o tests tests.c mm.c memlib.c
	./tests

clean:
	rm *.o
