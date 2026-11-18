CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

TARGET = matrix
SOURCES = main.cpp Matrix.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Флаги UnitTest++ определяются через pkg-config,
# при его отсутствии используется префикс Homebrew или системные пути.
UNITTEST_CFLAGS = $(shell pkg-config --cflags UnitTest++ 2>/dev/null)
UNITTEST_LIBS = $(shell pkg-config --libs UnitTest++ 2>/dev/null)

ifeq ($(UNITTEST_LIBS),)
    BREW_PREFIX = $(shell brew --prefix 2>/dev/null)

    ifeq ($(BREW_PREFIX),)
        UNITTEST_LIBS = -lUnitTest++
    else
        UNITTEST_CFLAGS = -I$(BREW_PREFIX)/include
        UNITTEST_LIBS = -L$(BREW_PREFIX)/lib -lUnitTest++
    endif
endif

TEST_TARGET = matrix_test
TEST_SOURCES = MatrixTest.cpp Matrix.cpp
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.test.o)
TEST_CXXFLAGS = $(CXXFLAGS) $(UNITTEST_CFLAGS)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CXX) $(TEST_CXXFLAGS) $(TEST_OBJECTS) -o $(TEST_TARGET) $(UNITTEST_LIBS)

%.test.o: %.cpp
	$(CXX) $(TEST_CXXFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET) $(TEST_OBJECTS) $(TEST_TARGET)

.PHONY: all run test clean
