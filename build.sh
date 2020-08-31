#!/bin/sh

BASEDIR=$(cd `dirname $0` && pwd)

# software
mkdir -p ${BASEDIR}/software/build
cd ${BASEDIR}/software/build
cmake ..
make all

# module
cd ${BASEDIR}/module
make all

# check module is loaded
if /sbin/lsmod | grep -q "vtty"; then
	echo "module loaded"
	make unload
	make load
else
	echo "module not loaded"
	make load
fi
