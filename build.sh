#!/bin/bash

# Parse command line arguments
CLEAN_ONLY=false
CLEAN_BEFORE_BUILD=false

for arg in "$@"; do
    case $arg in
        --clean)
            if [ $# -eq 1 ]; then
                CLEAN_ONLY=true
            else
                CLEAN_BEFORE_BUILD=true
            fi
            ;;
        -h|--help)
            echo "Usage: $0 [--clean] [--help]"
            echo "  --clean    Clean build cache (if alone) or clean before building"
            echo "  --help     Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown argument: $arg"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Navigate to BeamCommander directory
cd "$(dirname "$0")/openframeworks-src-master/apps/myApps/BeamCommander"

# Clean if requested
if [ "$CLEAN_ONLY" = true ] || [ "$CLEAN_BEFORE_BUILD" = true ]; then
    echo "Cleaning build cache..."
    make clean
    if [ $? -ne 0 ]; then
        echo "Clean failed!"
        exit 1
    fi
    echo "Clean completed!"
fi

# Exit if only cleaning was requested
if [ "$CLEAN_ONLY" = true ]; then
    exit 0
fi

# Build BeamCommander application
echo "Building BeamCommander..."

# Build with parallel compilation
make -j8

if [ $? -eq 0 ]; then
    echo "Build completed successfully!"
    echo "To run the application, use: ./start.sh"
else
    echo "Build failed!"
    exit 1
fi