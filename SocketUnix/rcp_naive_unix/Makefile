SOURCES := $(wildcard *.[ch])
TODAY   := $(shell date +%Y%m%d)
ARCHIVE := rcp_naive_unix_$(TODAY).zip
TARGETS := client server-iterative

CFLAGS := -std=c17 -O2 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 
LDFLAGS := -fpie -Wl,-pie

.PHONY: clean dist

all: $(TARGETS)

# dependencies
client.o: client.c utils.h
server-iterative.o: server-iterative.c utils.h
client: client.o utils.o
server-iterative: server-iterative.o utils.o

clean:
	@rm -rf $(TARGETS) *.zip *.o *.dSYM *~

dist:
	@zip $(ARCHIVE) $(SOURCES) Makefile
