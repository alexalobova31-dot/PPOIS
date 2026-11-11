CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
DEPFLAGS = -MMD -MP

TARGET = pharma
LIB_SOURCES = Product.cpp Organization.cpp Warehouse.cpp SalesDepartment.cpp Exceptions.cpp Common.cpp Catalog.cpp Commercial.cpp Orders.cpp StockManagement.cpp Delivery.cpp Company.cpp
SOURCES = main.cpp $(LIB_SOURCES)
OBJECTS = $(SOURCES:.cpp=.o)

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

TEST_TARGET = pharma_test
TEST_SOURCES = PharmaTest.cpp ExtendedPharmaTest.cpp $(LIB_SOURCES)
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.test.o)
TEST_CXXFLAGS = $(CXXFLAGS) $(UNITTEST_CFLAGS)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CXX) $(TEST_CXXFLAGS) $(TEST_OBJECTS) -o $(TEST_TARGET) $(UNITTEST_LIBS)

%.test.o: %.cpp
	$(CXX) $(TEST_CXXFLAGS) $(DEPFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

example: $(TARGET)
	./$(TARGET) --example

check:
	python3 tools/check_requirements.py

clean:
	rm -f $(OBJECTS) $(TARGET) $(TEST_OBJECTS) $(TEST_TARGET) $(OBJECTS:.o=.d) $(TEST_OBJECTS:.o=.d)

-include $(OBJECTS:.o=.d) $(TEST_OBJECTS:.o=.d)

.PHONY: all run test example check clean
