#!/bin/bash
# This script is used to build the C_COURSE project using CMake.
# It first checks if the build directory exists and removes it if it does.
# Then it creates a new build directory, navigates into it, and runs CMake to configure the project.
# Finally, it builds the project using make.
# Check if the build directory exists and remove it if it does
# Check if the script is being run from the correct directory

if [ -d "build" ]; then
    echo "Removing existing build directory..."
    rm -rf build
fi

mkdir build && cd build
cmake ..
make
cd ..