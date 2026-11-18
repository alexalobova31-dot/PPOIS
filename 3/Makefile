CXX = c++
CPPFLAGS ?=
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Werror -O2
LDFLAGS ?=
LDLIBS ?=
DEPFLAGS = -MMD -MP
TARGET = learning
TEST_TARGET = learning_test
LIB_SOURCES = Common.cpp Accounts.cpp Content.cpp Learning.cpp Community.cpp Finance.cpp Platform.cpp
OBJECTS = $(LIB_SOURCES:%.cpp=build/%.o)
UNITTEST_CFLAGS = $(shell pkg-config --cflags UnitTest++ 2>/dev/null)
UNITTEST_LIBS = $(shell pkg-config --libs UnitTest++ 2>/dev/null)
ifeq ($(UNITTEST_LIBS),)
UNITTEST_LIBS = -lUnitTest++
endif

.PHONY: all run test example sanitize clean
all: $(TARGET)

$(TARGET): build/main.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

build/%.o: %.cpp Makefile
	mkdir -p build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

build/LearningTest.o: CPPFLAGS += $(UNITTEST_CFLAGS)

$(TEST_TARGET): build/LearningTest.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LDLIBS) $(UNITTEST_LIBS) -o $@

run: $(TARGET)
	./$(TARGET)

example: $(TARGET)
	./$(TARGET) --example

test: $(TEST_TARGET)
	./$(TEST_TARGET)

sanitize:
	mkdir -p build
	$(CXX) $(CPPFLAGS) $(UNITTEST_CFLAGS) -std=c++17 -Wall -Wextra -Wpedantic -Werror -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer LearningTest.cpp $(LIB_SOURCES) $(LDFLAGS) $(LDLIBS) $(UNITTEST_LIBS) -o build/learning-sanitize
	./build/learning-sanitize

clean:
	$(RM) $(OBJECTS) $(OBJECTS:.o=.d) build/main.o build/main.d build/LearningTest.o build/LearningTest.d $(TARGET) $(TEST_TARGET) build/learning-sanitize build/learning

-include $(OBJECTS:.o=.d) build/main.d build/LearningTest.d
