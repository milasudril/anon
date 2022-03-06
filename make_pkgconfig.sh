#!/bin/bash

prefix=$1
output=$2

cat << pkgconfig > $output
prefix=$prefix
exec_prefix=\${prefix}
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: anon
Description: Simple library for working with anon files
Version: 1.0.0
Libs: -L\${libdir} -lanon
Cflags: -I\${includedir}
pkgconfig
