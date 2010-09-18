all: tp1 tests
tp1: tp1.o
	gcc -o tp1 tp1.o `pkg-config opencv --libs`
tp1.o: tp1.c histogram.h models.h stats.h
	gcc -g -o tp1.o -c tp1.c `pkg-config opencv --cflags`
tests: tests.c stats.h
	gcc -g -o tests tests.c `pkg-config opencv --libs` `pkg-config opencv --cflags`

clean:
	rm *.o
	rm tp1
	rm tests
