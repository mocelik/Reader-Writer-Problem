
CSTD=c11
all:
	gcc -o rwp main.c --std=$(CSTD) -lpthread

clean:
	rm rwp	