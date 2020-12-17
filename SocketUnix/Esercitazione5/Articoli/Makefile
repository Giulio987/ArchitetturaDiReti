JAVA_SOURCES := $(wildcard *.java)
JAVA_TARGETS := $(patsubst %.java,%.class,$(JAVA_SOURCES))
C_SOURCES    := $(wildcard *.[ch])
C_TARGETS    := lista_articoli lista_articoli_server
TODAY        := $(shell date +%Y%m%d)
ARCHIVE      := articoli_$(TODAY).zip

CFLAGS := -std=c17 -O2 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 # -march=native -mtune=native
LDFLAGS := -fpie -Wl,-pie 

ifeq ($(USE_LIBUNISTRING),yes)
CFLAGS += -DUSE_LIBUNISTRING=1
LDFLAGS += -lunistring
endif

.PHONY: clean dist

all: $(C_TARGETS) $(JAVA_TARGETS)

# dependencies
lista_articoli.o: lista_articoli.c utils.h rxb.h
lista_articoli_server.o: lista_articoli_server.c utils.h rxb.h
rxb.o: rxb.c rxb.h

lista_articoli: lista_articoli.o utils.o rxb.o
lista_articoli_server: lista_articoli_server.o utils.o rxb.o

%.class: %.java
	javac $<

clean:
	@rm -rf $(C_TARGETS) $(JAVA_TARGETS) *.zip *.o *.dSYM *~

dist:
	@zip $(ARCHIVE) $(C_SOURCES) $(JAVA_SOURCES) Makefile

