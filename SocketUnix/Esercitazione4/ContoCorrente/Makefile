JAVA_SOURCES := $(wildcard *.java)
JAVA_TARGETS := $(patsubst %.java,%.class,$(JAVA_SOURCES))
C_SOURCES    := $(wildcard *.[ch])
C_TARGETS    := conto_corrente_server controllo_conto_corrente
TODAY        := $(shell date +%Y%m%d)
ARCHIVE      := rps_$(TODAY).zip

CFLAGS := -std=c17 -O2 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 # -march=native -mtune=native
LDFLAGS := -fpie -Wl,-pie 

ifeq ($(USE_LIBUNISTRING),yes)
CFLAGS += -DUSE_LIBUNISTRING=1
LDFLAGS += -lunistring
endif

.PHONY: clean dist

all: $(C_TARGETS) $(JAVA_TARGETS)

# dependencies
controllo_conto_corrente.o: controllo_conto_corrente.c utils.h rxb.h
rxb.o: rxb.c rxb.h
conto_corrente_server.o: conto_corrente_server.c utils.h rxb.h

controllo_conto_corrente: controllo_conto_corrente.o utils.o rxb.o
conto_corrente_server: conto_corrente_server.o utils.o rxb.o

# client-naive: client-naive.o
# server-concurrent-naive: server-iterative-naive.o

%.class: %.java
	javac $<

clean:
	@rm -rf $(C_TARGETS) $(JAVA_TARGETS) *.zip *.o *.dSYM *~

dist:
	@zip $(ARCHIVE) $(C_SOURCES) $(JAVA_SOURCES) "Testo Esercizio.txt" README.txt Makefile

