# CONFIG {
target= pacman
compiler= g++
std= c++17
cflags= -Wno-narrowing
lflags= -pthread
libs= -lSDL2 -lSDL2main -lstdc++fs
# }
sources= $(wildcard src/*.cpp)
objects = $(sort $(patsubst src/%,obj/%, $(patsubst %.cpp,%.o,$(sources))))

.PHONY: untarget tidy clean run test

bin/$(target): $(objects)
	$(compiler) -std=$(std) -o bin/$(target) $(objects) $(lflags) $(libs)

obj/%.o: src/%.cpp
	$(compiler) -std=$(std) $(cflags) $(libs) -c -o $@ $^

untarget:
	rm -f bin/$(target)
tidy:
	rm -f $(objects)

clean: tidy untarget

run: clean bin/$(target)
	bin/$(target)

test: clean run
