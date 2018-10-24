.PHONY: all clean


CXX = g++
CXXFLAGS = -Wall -Wextra -Ofast -flto
SDLFLAGS = `sdl-config --cflags --libs` -lGL -lGLU

all: boids boids-openmp

boids: boid.h dmath.h gridhash.h lock.h painter.h gridhash.cc lock.cc main.cc
	$(CXX) $(CXXFLAGS) -Wno-unknown-pragmas -o boids *.cc $(SDLFLAGS) 

boids-openmp: boid.h dmath.h gridhash.h lock.h painter.h gridhash.cc lock.cc main.cc
	$(CXX) $(CXXFLAGS) -fopenmp -o boids-openmp *.cc $(SDLFLAGS)

clean:
	rm -f boids boids-openmp
