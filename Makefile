all: gbp2zhb 3sided nsided zheng-ball

CXXFLAGS=-std=c++17 -pedantic -Wall -O3 -DNDEBUG -g

GEOM=../libgeom
DFO=../dfo

INCLUDES=-I$(GEOM) -I$(DFO)
LIBS=-L$(GEOM)/release -L$(DFO)/build -lgeom -ldfo

% :: %.cc
	$(CXX) -o $@ $< $(CXXFLAGS) $(INCLUDES) $(LIBS)
