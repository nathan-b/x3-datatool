# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Werror -std=c++20
DBFLAGS := -g -O0 -DDEBUG
RELFLAGS := -O2
GTEST_CMAKE_FLAGS := -DCMAKE_CXX_STANDARD=20 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS=-D_GLIBCXX_USE_CXX11_ABI=1

# Directories
OUTDIR := build
OBJDIR := $(OUTDIR)/obj
GTEST_DIR := $(OUTDIR)/gtest
GTEST_BUILD := $(OUTDIR)/gtbuild

# Output binaries
BINARY := $(OUTDIR)/x3tool
TEST_BINARY := $(OUTDIR)/xttest

# Source files
MAIN_SRC := catdat.cpp
LIB_SRCS := operation.cpp datafile.cpp
TEST_SRCS := datafile.ut.cpp operation.ut.cpp
HEADERS := operation.h datafile.h datadir.h

# All sources (for dependency tracking)
ALL_SRCS := $(MAIN_SRC) $(LIB_SRCS) $(TEST_SRCS)
ALL_FORMAT_SRCS := $(ALL_SRCS) $(HEADERS)

# Object files
MAIN_OBJ := $(OBJDIR)/catdat.o
LIB_OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(LIB_SRCS))
TEST_OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(TEST_SRCS))

# Libraries and includes
GTEST_INCLUDES := -I$(abspath $(GTEST_DIR)/googletest/include)
LIBS :=
GTEST_LIB := $(GTEST_BUILD)/lib/libgtest.a
TEST_LIBS := $(GTEST_LIB) -lpthread

# Test data
TEST_DATA_DIR := test_artifacts
TEST_DATA := $(TEST_DATA_DIR)/test.cat $(TEST_DATA_DIR)/test.dat

# Enable parallel builds
MAKEFLAGS := --jobs=$(shell nproc)

# Default target
.PHONY: all
all: release

# Release build
.PHONY: release
release: CXXFLAGS += $(RELFLAGS)
release: $(BINARY)

# Debug build
.PHONY: debug
debug: CXXFLAGS += $(DBFLAGS)
debug: $(BINARY)

# Test build
.PHONY: test
test: CXXFLAGS += $(DBFLAGS)
test: $(TEST_BINARY)
	mkdir -p $(OUTDIR)/$(TEST_DATA_DIR)
	cp $(TEST_DATA) $(OUTDIR)/$(TEST_DATA_DIR)/

# Link main binary
$(BINARY): $(MAIN_OBJ) $(LIB_OBJS) | $(OUTDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Link test binary
$(TEST_BINARY): $(TEST_OBJS) $(LIB_OBJS) $(GTEST_LIB) | $(OUTDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_OBJS) $(LIB_OBJS) $(TEST_LIBS) $(LIBS)

# Compile main source to object
$(MAIN_OBJ): $(MAIN_SRC) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Compile library sources to objects
$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Compile test sources to objects
$(OBJDIR)/%.ut.o: %.ut.cpp $(GTEST_DIR) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(GTEST_INCLUDES) -c -o $@ $<

# Create directories
$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

# Google Test
$(GTEST_LIB): $(GTEST_DIR)
	mkdir -p $(GTEST_BUILD)
	cmake $(GTEST_CMAKE_FLAGS) -B $(GTEST_BUILD) $(GTEST_DIR)
	$(MAKE) -j -C $(GTEST_BUILD)

$(GTEST_DIR):
	git clone --depth=1 https://github.com/google/googletest.git $(GTEST_DIR)

# Clean
.PHONY: clean
clean:
	rm -rf $(OUTDIR)

# Run tests
.PHONY: run-tests
run-tests: test
	$(TEST_BINARY)

# Format code
.PHONY: format
format:
	clang-format -i $(ALL_FORMAT_SRCS)

# Help
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all         - Build release binary (default)"
	@echo "  release     - Build optimized release binary"
	@echo "  debug       - Build debug binary with symbols"
	@echo "  test        - Build test binary"
	@echo "  run-tests   - Build and run tests"
	@echo "  format      - Format all source files with clang-format"
	@echo "  clean       - Remove all build artifacts"
	@echo "  help        - Show this help message"
