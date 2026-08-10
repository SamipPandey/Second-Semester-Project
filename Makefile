# ============================================================================
# MAKEFILE: Dictionary Compressor/Decompressor
# WHAT THIS DOES: Defines compilation rules for building the project
# WHY AND INTENT: Automates the build process so users can compile with
#                  a single 'make' command instead of typing full g++ commands
# ============================================================================

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = compressor

# Source files
SOURCES = main.cpp compressor.cpp
HEADERS = compressor.h
OBJECTS = $(SOURCES:.cpp=.o)

# ============================================================================
# BUILD RULES
# ============================================================================

# Default target: build the executable
# WHAT THIS DOES: Compiles all source files and links them
# WHY AND INTENT: 'make' with no arguments builds everything
all: $(TARGET)

# Link object files into executable
# WHAT THIS DOES: Combines .o files into final binary
# WHY AND INTENT: Happens after all .cpp files are compiled to .o files
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)
	@echo "Build complete! Executable: $(TARGET)"

# Compile .cpp files to .o files
# WHAT THIS DOES: Translates C++ source to object files
# WHY AND INTENT: Separate compilation step allows faster rebuilds
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
# WHAT THIS DOES: Removes all .o files and executable
# WHY AND INTENT: Allows clean rebuild from scratch if needed
clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "Clean complete!"

# Rebuild everything from scratch
# WHAT THIS DOES: Runs clean then all
# WHY AND INTENT: Ensures all files are recompiled from source
rebuild: clean all

# Help target
help:
	@echo "Usage: make [target]"
	@echo "Targets:"
	@echo "  all      - Build the compressor (default)"
	@echo "  clean    - Remove build artifacts"
	@echo "  rebuild  - Clean and rebuild everything"
	@echo "  help     - Display this help message"

.PHONY: all clean rebuild help
