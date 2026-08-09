NPROC := $(shell nproc)
JOBS := $(shell echo $$(($(NPROC) - 2)))

dev:
	cmake -S . -B build/ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=OFF
	cmake --build build/ --parallel $(JOBS)
profile: dev
debug: dev

build:
	cmake -S . -B build/ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release -DENABLE_ASAN=OFF
	cmake --build build/ --parallel $(JOBS)

native:
	cmake -S . -B build/ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release -DENABLE_ASAN=OFF -DCMAKE_CXX_FLAGS="-march=native"
	cmake --build build/ --parallel $(JOBS)

nativedbg:
	cmake -S . -B build/ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=OFF -DCMAKE_CXX_FLAGS="-march=native"
	cmake --build build/ --parallel $(JOBS)

runndbg: nativedbg
	build/uniflashtool

run: native
	cmake -S . -B build/ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release -DENABLE_ASAN=OFF
	build/uniflashtool

.PHONY: dev debug build clean run native nativedbg runndbg