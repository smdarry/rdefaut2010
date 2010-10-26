HEADERS = blob.h histogram.h models.h stats.h utils.h etiquette.h tracking.h

all: tp1 tp2 tp3 tests
tp1: tp1.o
	gcc -o tp1 tp1.o `pkg-config opencv --libs`
tp1.o: tp1.c $(HEADERS)
	gcc -g -o tp1.o -c tp1.c `pkg-config opencv --cflags`
tests: tests.c $(HEADERS)
	gcc -g -o tests tests.c etiquette.o `pkg-config opencv --libs` `pkg-config opencv --cflags`

tp2: tp2.o etiquette.o
	gcc -o tp2 tp2.o etiquette.o `pkg-config opencv --libs`
tp2.o: tp2.c $(HEADERS)
	gcc -g -o tp2.o -c tp2.c `pkg-config opencv --cflags`
etiquette.o: etiquette.c etiquette.h
	gcc -g -o etiquette.o -c etiquette.c `pkg-config opencv --cflags`

tp3: tp3.o etiquette.o
	gcc -o tp3 tp3.o etiquette.o `pkg-config opencv --libs`
tp3.o: tp3.c $(HEADERS)
	gcc -g -o tp3.o -c tp3.c `pkg-config opencv --cflags`

# Utilitaires
nettoyage: nettoyage.c
	gcc -g -o nettoyage nettoyage.c `pkg-config opencv --libs` `pkg-config opencv --cflags`
bbox: bbox.c blob.h
	gcc -g -o bbox bbox.c etiquette.o `pkg-config opencv --libs` `pkg-config opencv --cflags`

clean:
	rm *.o
	rm tp?
	rm tests
