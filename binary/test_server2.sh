#!/bin/bash 
fuser -k 6001/tcp
clear
gdb -ex=r --args tree-server 6001 localhost:2181