JAVA_SOURCES := $(wildcard *.java)
JAVA_TARGETS := $(patsubst %.java,%.class,$(JAVA_SOURCES))
C_SOURCES    := $(wildcard *.[ch])
C_TARGETS    := client-naive client-lpd-connreuse server-concurrent-naive server-concurrent-lpd-connreuse-tmpfile server-concurrent-lpd-connreuse-buffer
TODAY        := $(shell date +%Y%m%d)
ARCHIVE      := ultima_lezione_$(TODAY).zip

CFLAGS := -std=c17 -O2 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 # -march=native -mtune=native
LDFLAGS := -fpie -Wl,-pie 

ifeq ($(USE_LIBUNISTRING),yes)
CFLAGS += -DUSE_LIBUNISTRING=1
LDFLAGS += -lunistring
endif

.PHONY: clean dist

all: $(C_TARGETS) $(JAVA_TARGETS)

# dependencies
client-lpd-connreuse.o: client-lpd-connreuse.c utils.h
server-concurrent-lpd-connreuse-tmpfile.o: server-concurrent-lpd-connreuse-tmpfile.c utils.h
server-concurrent-lpd-connreuse-buffer.o: server-concurrent-lpd-connreuse-buffer.c utils.h

client-lpd-connreuse: client-lpd-connreuse.o utils.o
server-concurrent-lpd-connreuse-tmpfile: server-concurrent-lpd-connreuse-tmpfile.o utils.o
server-concurrent-lpd-connreuse-buffer: server-concurrent-lpd-connreuse-buffer.o utils.o

# implicit dependencies
# client-naive: client-naive.o
# server-concurrent-naive: server-iterative-naive.o

%.class: %.java
	javac $<

clean:
	@rm -rf $(C_TARGETS) $(JAVA_TARGETS) *.zip *.o *.dSYM *~

dist:
	@zip $(ARCHIVE) $(C_SOURCES) $(JAVA_SOURCES) Makefile README.txt superautomatiche.txt \
		"Compito di Reti di Calcolatori del 9_7_2019 - errata corrige.pdf" 

