all: tp1 tp2 tests nettoyage
tp1: tp1.o
	gcc -o tp1 tp1.o `pkg-config opencv --libs`
tp1.o: tp1.c histogram.h models.h stats.h utils.h
	gcc -g -o tp1.o -c tp1.c `pkg-config opencv --cflags`
tests: tests.c stats.h
	gcc -g -o tests tests.c `pkg-config opencv --libs` `pkg-config opencv --cflags`
nettoyage: nettoyage.c
	gcc -g -o nettoyage nettoyage.c `pkg-config opencv --libs` `pkg-config opencv --cflags`

tp2: tp2.o etiquette.o
	gcc -o tp2 tp2.o etiquette.o `pkg-config opencv --libs`
tp2.o: tp2.c histogram.h blob.h etiquette.h
	gcc -g -o tp2.o -c tp2.c `pkg-config opencv --cflags`
etiquette.o: etiquette.c etiquette.h
	gcc -g -o etiquette.o -c etiquette.c `pkg-config opencv --cflags`

clean:
	rm *.o
	rm tp1
	rm tp2
	rm tests
