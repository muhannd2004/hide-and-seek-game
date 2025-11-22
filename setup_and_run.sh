#!/bin/bash

echo "🎮 Hide and Seek Game Setup"

# Install dependencies
echo "📦 Installing dependencies..."
sudo apt-get update
sudo apt-get install -y libraylib-dev libglpk-dev libgl1-mesa-dev

# Compile
echo "🔨 Compiling..."
gcc main.c menu.c gui.c simulate.c solve.c -o hideseek \
    -lraylib -lm -lpthread -ldl -lrt -lGL -lX11 -lGLU -lglpk

# Run
echo "🚀 Starting game..."
./hideseek
