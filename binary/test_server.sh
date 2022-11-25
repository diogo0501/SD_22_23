#!/bin/bash 
clear
cd ..
make
cd binary/
gdb -ex=r --args tree-server 6000 localhost:2181