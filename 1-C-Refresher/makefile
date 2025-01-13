# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -g

# Target executable name
TARGET = stringfun

# Default target
all: $(TARGET)

# Compile source to executable
$(TARGET): stringfun.c
	$(CC) $(CFLAGS) -o $(TARGET) $^

# Clean up build files
clean:
	rm -f $(TARGET)

# Perform a leak check with Valgrind
lc:
	valgrind --tool=memcheck --leak-check=yes --track-origins=yes ./stringfun -w "This is a long string"

# Phony targets
.PHONY: all clean
