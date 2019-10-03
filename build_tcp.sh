#!/bin/bash
BUILD_DIR=cmake-build-debug
#mkdir -p $BUILD_DIR
#cmake -DCMAKE_BUILD_TYPE=Debug -G "CodeBlocks - Unix Makefiles" /home/danila/git/NetworksLab2019/$BUILD_DIR
CLION_CMAKE="$HOME/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/192.6817.18/bin/cmake/linux/bin/cmake"
$CLION_CMAKE --build $BUILD_DIR --target my_client_linux \
  --target my_server_linux \
  -- -j 4