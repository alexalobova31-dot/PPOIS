CXX = c++
CPPFLAGS ?=
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Werror -O2
LDFLAGS ?=
UNITTEST_CFLAGS = $(shell pkg-config --cflags UnitTest++ 2>/dev/null)
UNITTEST_LIBS = $(shell pkg-config --libs UnitTest++ 2>/dev/null)
ifeq ($(UNITTEST_LIBS),)
UNITTEST_LIBS = -lUnitTest++
endif
HEADERS = Graph.h Sorting.h Student.h Console.h
COVERAGE_FILES = $(addprefix $(CURDIR)/,Graph.h Sorting.h Student.h Console.cpp)
COVERAGE_CXX ?= clang++
LLVM_COV ?= $(shell command -v llvm-cov 2>/dev/null || xcrun --find llvm-cov)
LLVM_PROFDATA ?= $(shell command -v llvm-profdata 2>/dev/null || xcrun --find llvm-profdata)

.PHONY: all run test clean coverage sanitize
all: lab4

lab4: main.cpp Console.cpp $(HEADERS) Makefile
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) main.cpp Console.cpp $(LDFLAGS) -o $@

lab4_test: LabTest.cpp Console.cpp $(HEADERS) Makefile
	$(CXX) $(CPPFLAGS) $(UNITTEST_CFLAGS) $(CXXFLAGS) LabTest.cpp Console.cpp $(LDFLAGS) $(UNITTEST_LIBS) -o $@

run: lab4
	./lab4

test: lab4_test
	./lab4_test

coverage:
	mkdir -p build
	$(COVERAGE_CXX) $(CPPFLAGS) $(UNITTEST_CFLAGS) -std=c++17 -Wall -Wextra -Wpedantic -Werror -O0 -g -fprofile-instr-generate -fcoverage-mapping LabTest.cpp Console.cpp $(LDFLAGS) $(UNITTEST_LIBS) -o build/coverage-test
	LLVM_PROFILE_FILE=build/tests.profraw ./build/coverage-test
	$(LLVM_PROFDATA) merge -sparse build/tests.profraw -o build/tests.profdata
	$(LLVM_COV) report build/coverage-test -instr-profile=build/tests.profdata $(COVERAGE_FILES) > build/coverage.txt
	cat build/coverage.txt
	awk '$$1 == "TOTAL" { found = 1; passed = ($$10 + 0 > 90) } END { exit !(found && passed) }' build/coverage.txt
	$(LLVM_COV) show build/coverage-test -instr-profile=build/tests.profdata -format=html -output-dir=build/coverage $(COVERAGE_FILES)

sanitize:
	mkdir -p build
	$(CXX) $(CPPFLAGS) $(UNITTEST_CFLAGS) -std=c++17 -Wall -Wextra -Wpedantic -Werror -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer LabTest.cpp Console.cpp $(LDFLAGS) $(UNITTEST_LIBS) -o build/sanitize-test
	./build/sanitize-test

clean:
	$(RM) -r build lab4 lab4_test
