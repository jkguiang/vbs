# Simple makefile

EXE=bin/$(study)
MAINDIR=.

SRCDIR=studies/$(study)
OBJDIR=studies/$(study)

# SOURCES=$(wildcard $(SRCDIR)/*.cc) $(wildcard tools/*.cc) $(wildcard Tools/*.cc) $(wildcard Tools/btagsf/*.cc) $(wildcard Tools/jetcorr/PhysicsTools/NanoAODTools/src/*.cc)
SOURCES=$(wildcard $(SRCDIR)/*.cc) $(wildcard tools/*.cc)
OBJECTS=$(SOURCES:%.cc=%.o)
HEADERS=$(SOURCES:.cc=.h)

CC          = g++
CXX         = g++
CXXFLAGS    = -g -O2 -Wall -fPIC -Wshadow -Woverloaded-virtual
LD          = g++
LDFLAGS     = -g -O2
SOFLAGS     = -g -shared
CXXFLAGS    = -g -O2 -Wall -fPIC -Wshadow -Woverloaded-virtual
LDFLAGS     = -g -O2 -L$(MAINDIR)/rapido/src -L$(MAINDIR)/NanoTools/NanoCORE -L${CMSSW_BASE}/lib/${SCRAM_ARCH} -L/cvmfs/cms.cern.ch/slc7_amd64_gcc900/external/py3-correctionlib/2.0.0-0c4f44c8dd5561d8c0660135feeb81f4/lib/python3.9/site-packages/correctionlib/lib
# LDFLAGS     = -g -O2 -L$(MAINDIR)/rapido/src -L$(MAINDIR)/NanoTools/NanoCORE -L${CMSSW_BASE}/lib/${SCRAM_ARCH}
ROOTLIBS    = $(shell root-config --libs)
ROOTCFLAGS  = $(shell root-config --cflags)
CXXFLAGS   += $(ROOTCFLAGS)
CFLAGS      = $(ROOTCFLAGS) -Wall -Wno-unused-function -g -O2 -fPIC -fno-var-tracking
CFLAGS     += -I$(MAINDIR)/rapido/src -I$(MAINDIR)/NanoTools/NanoCORE -I${CMSSW_BASE}/src -I${CMSSW_BASE}/../../../external/boost/1.67.0/include -I/cvmfs/cms.cern.ch/slc7_amd64_gcc900/external/py3-correctionlib/2.0.0-0c4f44c8dd5561d8c0660135feeb81f4/lib/python3.9/site-packages/correctionlib/include
# CFLAGS     += -I$(MAINDIR)/rapido/src -I$(MAINDIR)/NanoTools/NanoCORE -I${CMSSW_BASE}/src -I${CMSSW_BASE}/../../../external/boost/1.67.0/include
EXTRAFLAGS  = -fPIC -ITMultiDrawTreePlayer -Wunused-variable -lTMVA -lEG -lGenVector -lXMLIO -lMLP -lTreePlayer
EXTRAFLAGS += -lRAPIDO -lNANO_CORE -lCondFormatsJetMETObjects -lJetMETCorrectionsModules -lcorrectionlib

all: $(EXE)

$(EXE): $(OBJECTS)
	$(LD) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) $(ROOTLIBS) $(EXTRAFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f $(OBJECTS) $(EXE)

.PHONY: all
