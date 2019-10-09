#!/bin/bash

rm client_linux/client_linux
rm server_linux/server_linux
rm client
rm server

cmake client_linux/CMakeLists.txt
cmake server_linux/CMakeLists.txt

cd client_linux
make
cd ..

cd server_linux
make
cd ..

cp client_linux/client_linux client
cp server_linux/server_linux server
