CXX = g++
LD = g++
CXXFLAGS = -std=c++17 -I./inc -I./cul/inc -g -Wall -pedantic -Werror -DMACRO_PLATFORM_LINUX
SOURCES  = $(shell find src | grep '[.]cpp$$')
OBJECTS_DIR = .debug-build
OUTPUT = libecs-d.a
OBJECTS = $(addprefix $(OBJECTS_DIR)/,$(SOURCES:%.cpp=%.o))

$(OBJECTS_DIR)/%.o: | $(OBJECTS_DIR)/src
	$(CXX) $(CXXFLAGS) -c $*.cpp -o $@

default: $(OBJECTS)
	@echo $(SOURCES)
	ar rvs $(OUTPUT) $(OBJECTS)

$(OBJECTS_DIR)/src:
	mkdir -p $(OBJECTS_DIR)/src

clean:
	rm -rf $(OBJECTS_DIR)

libecs-d.a : default

test: $(OUTPUT)
	$(CXX) $(CXXFLAGS) -L$(shell pwd) ecs_test.cpp -lecs-d -o ecs_test
	./ecs_test
