#!/bin/bash
cd /src
premake5 gmake 
cd build 
make -j 3 config=release_x64 
mkdir /rd2l 
cp ./bin/x64/Release/rd2lbot /rd2l/rd2lbot
