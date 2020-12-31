JAVA_SOURCES := $(wildcard *.java)
JAVA_TARGETS := $(patsubst %.java,%.class,$(JAVA_SOURCES))
C_SOURCES    := $(wildcard *.[ch])
C_TARGETS    := largest_expenses largest_expenses_server
TODAY        := $(shell date +%Y%m%d)
ARCHIVE      := 144195_Milani_Giulio.zip

CFLAGS := -std=c17 -O2 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 # -march=native -mtune=native
LDFLAGS := -fpie -Wl,-pie 

ifeq ($(USE_LIBUNISTRING),yes)
CFLAGS += -DUSE_LIBUNISTRING=1
LDFLAGS += -lunistring
endif

.PHONY: clean dist

all: $(C_TARGETS) $(JAVA_TARGETS)

# dependencies
largest_expenses.o: largest_expenses.c utils.h rxb.h
rxb.o: rxb.c rxb.h
largest_expenses_server.o: largest_expenses_server.c utils.h rxb.h

largest_expenses: largest_expenses.o utils.o rxb.o
largest_expenses_server: largest_expenses_server.o utils.o rxb.o

%.class: %.java
	javac $<

clean:
	@rm -rf $(C_TARGETS) $(JAVA_TARGETS) *.zip *.o *.dSYM *~

dist:
	@zip $(ARCHIVE) $(C_SOURCES) $(JAVA_SOURCES)  Makefile

