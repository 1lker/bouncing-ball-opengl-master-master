# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Iinclude -I/opt/homebrew/Cellar/glfw/3.4/include -Wall -O2 -DGL_SILENCE_DEPRECATION
LDFLAGS = -L/opt/homebrew/Cellar/glfw/3.4/lib

SRCDIR = src
SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(SOURCES:.cpp=.o)

TARGET = EnhancedBouncingBall

LIBS = $(LDFLAGS) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -lglfw

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
