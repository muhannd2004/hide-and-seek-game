# ============================================================================
# Hide and Seek Game - Makefile
# ============================================================================

# Compiler settings
CC := gcc
CFLAGS := -Wall -Wextra -O2
LIBS := -lraylib -lm -lpthread -ldl -lrt -lGL -lX11 -lGLU -lglpk
TARGET := hideseek

# Auto-detect if files are organized or not
ifneq (,$(wildcard src/main.c))
    # Files are in src/ structure
    SRCS = src/main.c src/ui/menu.c src/ui/gui.c src/core/simulate.c src/ai/solve.c
    INCLUDE = -Iinclude
    $(info ✓ Using organized structure: src/)
else
    # Files are in root directory
    SRCS = main.c menu.c gui.c simulate.c solve.c
    INCLUDE = 
    $(info ✓ Using flat structure: root directory)
endif

# Colors for output
GREEN := \033[0;32m
YELLOW := \033[0;33m
RED := \033[0;31m
NC := \033[0m # No Color

# ============================================================================
# Main targets
# ============================================================================

.PHONY: all clean run debug install help organize

# Default target - build and run
all: $(TARGET)
	@echo "✅ Build complete! Starting game..."
	@./$(TARGET)

# Build only
$(TARGET): $(SRCS)
	@echo "🔨 Compiling $(TARGET)..."
	$(CC) $(CFLAGS) $(INCLUDE) $(SRCS) -o $(TARGET) $(LIBS)

# Run without rebuilding
run: $(TARGET)
	@echo "🚀 Running game..."
	./$(TARGET)

# Clean build artifacts
clean:
	@echo "🧹 Cleaning build files..."
	rm -f $(TARGET)
	@rm -f scores_*.txt
	@echo "$(GREEN)✓ Clean complete$(NC)"

# Check dependencies
check:
	@echo "$(YELLOW)→ Checking dependencies...$(NC)"
	@command -v gcc >/dev/null 2>&1 || { echo "$(RED)✗ GCC not found$(NC)"; exit 1; }
	@pkg-config --exists raylib || { echo "$(RED)✗ Raylib not found$(NC)"; exit 1; }
	@pkg-config --exists glpk || { echo "$(RED)✗ GLPK not found$(NC)"; exit 1; }
	@command -v python3 >/dev/null 2>&1 || { echo "$(RED)✗ Python3 not found$(NC)"; exit 1; }
	@echo "$(GREEN)✓ All dependencies found$(NC)"

# Display help
help:
	@echo "$(GREEN)Hide and Seek Game - Makefile Help$(NC)"
	@echo ""
	@echo "$(YELLOW)Usage:$(NC)"
	@echo "  make           - Build and run immediately (default)"
	@echo "  make run       - Run existing build"
	@echo "  make debug     - Build with debug symbols and start gdb"
	@echo "  make quick     - Clean rebuild and run"
	@echo "  make clean     - Remove build artifacts"
	@echo "  make install   - Install system-wide (requires sudo)"
	@echo "  make check     - Verify all dependencies"
	@echo "  make organize  - Organize files into src/ structure"
	@echo "  make help      - Show this help message"

# ============================================================================
# Advanced targets
# ============================================================================

# Show build info
info:
	@echo "$(GREEN)Build Information:$(NC)"
	@echo "  Compiler: $(CC)"
	@echo "  Flags: $(CFLAGS)"
	@echo "  Sources: $(words $(SRCS)) files"
	@echo "  Target: $(TARGET)"

# Count lines of code
stats:
	@echo "$(GREEN)Project Statistics:$(NC)"
	@echo "  Total lines:"
	@wc -l $(SRCS) $(wildcard include/*.h) | tail -1
	@echo "  Source files: $(words $(SRCS))"
	@echo "  Header files: $(words $(wildcard include/*.h))"

# Run with valgrind for memory leaks
valgrind: $(TARGET)
	@echo "$(YELLOW)→ Running with Valgrind...$(NC)"
	@valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

# Create release package
release: clean
	@echo "$(YELLOW)→ Creating release package...$(NC)"
	@mkdir -p release
	@tar -czf release/hideseek-$(shell date +%Y%m%d).tar.gz \
		$(SRC_DIR) $(INCLUDE_DIR) resources Makefile README.md
	@echo "$(GREEN)✓ Release package created$(NC)"
