CFLAGS=-I. -g -Wall
LDFLAGS=-lmxml

tcxp: sax-maker.o tcxparser.o
	gcc -o tcxp $^ $(LDFLAGS)
