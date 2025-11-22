#!/bin/bash
# Complete setup script

echo "🎮 Setting up Hide and Seek Game..."

# Make scripts executable
chmod +x run.sh organize.sh

# Check dependencies
echo "📦 Checking dependencies..."
./Makefile check || {
    echo "❌ Missing dependencies. Installing..."
    sudo apt-get update
    sudo apt-get install -y libraylib-dev libglpk-dev python3-matplotlib
}

# Organize files (if not already organized)
if [ -f "solve.c" ]; then
    echo "📂 Organizing files..."
    ./organize.sh
fi

# Build project
echo "🔨 Building project..."
make clean
make

echo "✅ Setup complete! Run with: make"
