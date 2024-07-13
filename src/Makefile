CXX = g++ -g -Wall
CXXFLAGS = -std=c++17
LDFLAGS_MAIN = -pthread
TARGET = BlueNap
SRCS := $(wildcard src/*.cpp)
OBJS := $(filter-out build/main.o, $(patsubst src/%.cpp,build/%.o,$(SRCS)))
MAIN_OBJ := build/main.o

all: $(TARGET)

$(TARGET): $(OBJS) $(MAIN_OBJ)
	$(CXX) $^ -o $(TARGET)

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/main.o: src/main.cpp | build
	$(CXX) $(CXXFLAGS) $(LDFLAGS_MAIN) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build $(TARGET)

run: $(TARGET)
	./$(TARGET)