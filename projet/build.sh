#!/bin/bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
echo "✅ Build terminé: ./build/main"