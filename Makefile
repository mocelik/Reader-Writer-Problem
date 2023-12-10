
CSTD=c11
all: rwp

rwp:
	gcc -o rwp main.c --std=$(CSTD) -lpthread

clean:
	rm rwp	