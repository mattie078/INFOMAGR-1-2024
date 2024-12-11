CC = g++
PROGRAM_NAME = rtweekend.exe
SOURCES = $(wildcard *.cpp)
OBJS = $(SOURCES:.cpp=.o)
DEPS = 
CPPFLAGS = -Wall -Wextra -fopenmp -Wno-unused-parameter -I./SDL2/include -L./SDL2/lib -lSDL2

.PHONY: all clean distclean run valgrind

all: clean $(PROGRAM_NAME)

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)

$(PROGRAM_NAME): $(OBJS)
	$(CC) $^ $(CPPFLAGS) -o $(PROGRAM_NAME)

run: $(PROGRAM_NAME)
	./$(PROGRAM_NAME)

valgrind: $(PROGRAM_NAME)
	valgrind --suppressions=valgrind-suppressions.supp --leak-check=yes ./$(PROGRAM_NAME)

clean:
	del $(PROGRAM_NAME)
	del *.o

distclean: clean
