# Get list of all *.java source code files, create corresponding *.class target list.
SOURCES := $(wildcard *.java)
OBJECTS := $(notdir ${SOURCES})
TARGETS := $(patsubst %.java,%.class,$(OBJECTS))

TODAY   := $(shell date +%Y%m%d)
ARCHIVE := rcp_java_$(TODAY).zip


.PHONY: clean dist

all: ${TARGETS}

%.class: %.java
	@javac $<

clean:
	@rm -f $(ARCHIVE) *.class *~

dist:
	@zip $(ARCHIVE) $(SOURCES) Makefile

