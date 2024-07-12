all: build

env:
	./scripts/get_dependencies.sh

build:
	@if [ ! -d ${basedir}/build ]; then\
  	cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi-rpi1.cmake;\
	fi

	cmake --build build

b: build

build_test:
	@if [ ! -d ${basedir}/build_test ]; then\
  	cmake -S . -B build_test -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON -DPREDOS_TEST=1 -DCMAKE_C_COMPILER=/usr/bin/gcc;\
	fi

	cmake --build build_test

bt: build_test

run_test:
	ctest --output-on-failure --verbose --test-dir build_test

rt: run_test

coverage: run_test
	mkdir coverage
	python3 -m gcovr -r src/kernel/fat --object-directory build_test/src/kernel/fat --html-details coverage/report.html

c: coverage

clean:
	rm -rf build build_test coverage

.PHONY: build build_test run_test coverage b bt rt c

