CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O3 -flto -DNDEBUG

SOURCE_DIR := src
OUTPUT_DIR := build
EXECUTABLE := $(OUTPUT_DIR)/cclox

HEADERS := $(wildcard $(SOURCE_DIR)/*.h)
SOURCES := $(wildcard $(SOURCE_DIR)/*.cpp)
OBJECTS := $(addprefix $(OUTPUT_DIR)/, $(notdir $(SOURCES:.cpp=.o)))

default: prebuild $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@ echo $@
	@ $(CXX) $(CXXFLAGS) $^ -o $@

$(OUTPUT_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(HEADERS)
	@ echo $@
	@ $(CXX) -c $(CXXFLAGS) $< -o $@

prebuild:
	@ mkdir -p $(OUTPUT_DIR)

clean:
	@ rm -rf $(OUTPUT_DIR)

run:
	@ $(EXECUTABLE)

.PHONY: default prebuild clean run
