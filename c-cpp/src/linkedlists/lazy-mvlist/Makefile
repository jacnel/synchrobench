ROOT = ../../..

include $(ROOT)/common/Makefile.common

$(shell [ -d "$(BUILDIR)/unsafe" ] || mkdir -p $(BUILDIR)/unsafe)

BINS = $(BINDIR)/$(LOCK)-lazy-mvlist
CFLAGS += -std=gnu89

ifeq ($(NOPREALLOC), TRUE)
	CFLAGS += -DNOPREALLOC
endif

ifeq ($(PAD), TRUE)
	CFLAGS += -DPAD
endif

ifeq ($(COUNTRQ), TRUE)
	CFLAGS += -DCOUNT_RQ
endif

.PHONY:	all clean

all:	main

%.o: %.c
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/$@ $<

main: node.o rqtracker.o arena.o lazy.o intset.o unsafe/node-unsafe.o unsafe/lazy-unsafe.o unsafe/intset-unsafe.o test.o
	$(CC) $(CFLAGS) $(BUILDIR)/node.o $(BUILDIR)/unsafe/node-unsafe.o $(BUILDIR)/rqtracker.o $(BUILDIR)/arena.o $(BUILDIR)/lazy.o $(BUILDIR)/unsafe/lazy-unsafe.o $(BUILDIR)/intset.o $(BUILDIR)/unsafe/intset-unsafe.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	rm -f $(BINS)