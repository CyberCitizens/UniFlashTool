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

.PHONY: dev debug build clean