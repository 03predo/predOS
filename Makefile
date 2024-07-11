all: build

env:
	./scripts/get_dependencies.sh

build:
	@if [ ! -d ${basedir}/build ]; then\
  	cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi-rpi1.cmake;\
	fi

	cmake --build build

build_test:
	@if [ ! -d ${basedir}/build_test ]; then\
  	cmake -S . -B build_test -DPREDOS_TEST=1;\
	fi

	cmake --build build_test

run_test:
	ctest --output-on-failure --verbose --test-dir build_test


.PHONY: build build_test

