ROOT = ../../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/$(LOCK)-lazy-mvlist
CFLAGS += -std=gnu89

.PHONY:	all clean

all:	main

node.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/node.o node.c

rqtracker.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/rqtracker.o rqtracker.c

arena.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/arena.o arena.c

lazy.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/lazy.o lazy.c	 

intset.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o intset.c

test.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o test.c

main: node.o rqtracker.o arena.o lazy.o intset.o test.o
	$(CC) $(CFLAGS) $(BUILDIR)/node.o $(BUILDIR)/rqtracker.o $(BUILDIR)/arena.o $(BUILDIR)/lazy.o $(BUILDIR)/intset.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	rm -f $(BINS)
