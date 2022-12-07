#!/bin/bash 
fuser -k $1/tcp
clear
cd ..
make
cd binary/
./tree-server $1 localhost:$2