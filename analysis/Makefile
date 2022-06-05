# Simple makefile

EXE=bin/$(study)
MAINDIR=.

SRCDIR=studies/$(study)
OBJDIR=studies/$(study)

SOURCES=$(wildcard $(SRCDIR)/*.cc)
OBJECTS=$(SOURCES:$(SRCDIR)/%.cc=$(OBJDIR)/%.o)
HEADERS=$(SOURCES:.cc=.h)

CC          = g++
CXX         = g++
CXXFLAGS    = -g -O2 -Wall -fPIC -Wshadow -Woverloaded-virtual
LD          = g++
LDFLAGS     = -g -O2
SOFLAGS     = -g -shared
CXXFLAGS    = -g -O2 -Wall -fPIC -Wshadow -Woverloaded-virtual
LDFLAGS     = -g -O2 -L$(MAINDIR)/rapido/src -L$(MAINDIR)/NanoTools/NanoCORE
ROOTLIBS    = $(shell root-config --libs)
ROOTCFLAGS  = $(shell root-config --cflags)
CXXFLAGS   += $(ROOTCFLAGS)
CFLAGS      = $(ROOTCFLAGS) -Wall -Wno-unused-function -g -O2 -fPIC -fno-var-tracking
CFLAGS     += -I$(MAINDIR)/rapido/src -I$(MAINDIR)/NanoTools/NanoCORE
EXTRAFLAGS  = -fPIC -ITMultiDrawTreePlayer -Wunused-variable -lTMVA -lEG -lGenVector -lXMLIO -lMLP -lTreePlayer
EXTRAFLAGS += -lRAPIDO -lNANO_CORE

all: $(EXE)

$(EXE): $(OBJECTS)
	$(LD) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) $(ROOTLIBS) $(EXTRAFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f $(OBJECTS) $(EXE)

.PHONY: all
