#!/bin/bash
# Quick run script - just execute ./run.sh

set -e

echo "🎮 Hide and Seek Game - Quick Start"
echo ""

# Check if make exists
if ! command -v make &> /dev/null; then
    echo "❌ Make not found, compiling directly..."
    gcc main.c menu.c gui.c simulate.c solve.c -o hideseek \
        -lraylib -lm -lpthread -ldl -lrt -lGL -lX11 -lGLU -lglpk
    ./hideseek
else
    make all
fi
