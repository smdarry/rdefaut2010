all: tp1
tp1: tp1.o
	gcc -o tp1 tp1.o `pkg-config opencv --libs`
tp1.o: tp1.c
	gcc -g -o tp1.o -c tp1.c `pkg-config opencv --cflags`

clean:
	rm *.o
	rm tp1
