#!/bin/bash 
clear
cd ..
make
cd binary/
gdb -tui -ex=r --args tree-server 6000 localhost:2181