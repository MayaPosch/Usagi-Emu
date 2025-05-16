
CXX ?= g++
CC ?= gcc
STRIP = strip
MAKEDIR = mkdir -p
RM = rm

OUTPUT := ue1_emu

SOURCES := $(wildcard *.cpp)
OBJECTS := $(addprefix obj/,$(notdir) $(SOURCES:.cpp=.o))

CXXFLAGS := $(INCLUDE) $(CXXFLAGS) -g3 -std=c++14 -O0


all: makedir bin/$(OUTPUT)

makedir:
	$(MAKEDIR) bin
	$(MAKEDIR) obj

obj/%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)
	
bin/$(OUTPUT): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LIB)

clean:
	$(RM) $(OBJECTS)
	