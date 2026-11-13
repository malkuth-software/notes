# Builds the playground driver and the three engines into build/bin, each with
# its upstream flags. Run from this folder: make, then build/bin/tracer_bakes.
CXX ?= c++
CC ?= cc
BFFT := engines/bfft/experiments/photonic_transport_field/native

.PHONY: all run clean
all: build/bin/tracer_bakes build/bin/teapot_bake build/bin/lamp_keys build/bin/regime_scene_native

build/bin:
	mkdir -p build/bin

# The playground driver and its own column.
build/bin/tracer_bakes: tracer_bakes.cpp scenes.hpp | build/bin
	$(CXX) -std=c++20 -O2 -ffp-contract=off -pthread -o $@ tracer_bakes.cpp

# conv-teapot (its build.sh flags) through the adapter.
build/bin/teapot_bake: teapot_bake.cpp scenes.hpp $(wildcard engines/conv_teapot/src/*.hpp) | build/bin
	$(CXX) -std=c++20 -O3 -ffp-contract=off -pthread -o $@ teapot_bake.cpp

# lamp_keys (the industrial-surfaces README command).
build/bin/lamp_keys: engines/lamp_keys/lamp_keys.cpp | build/bin
	$(CXX) -std=c++20 -O2 -pthread -o $@ engines/lamp_keys/lamp_keys.cpp

# bfft regime_scene_native (its Makefile flags).
build/conv_native.o: engines/bfft/standalone_conv_resize_demo/native/conv_native.c | build/bin
	$(CC) -O3 -DNDEBUG -std=c11 -pthread -mcpu=native -c $< -o $@

build/bin/regime_scene_native: $(wildcard $(BFFT)/*.cpp $(BFFT)/*.hpp $(BFFT)/*.h) build/conv_native.o | build/bin
	$(CXX) -O3 -DNDEBUG -std=c++20 -mcpu=native $(BFFT)/regime_scene_native.cpp $(BFFT)/retained_transport.cpp build/conv_native.o -pthread -o $@

run: all
	build/bin/tracer_bakes 160

clean:
	rm -rf build
