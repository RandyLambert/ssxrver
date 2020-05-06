#!/bin/bash

set -x

SOURCE_DIR=`pwd`
BUILD_NAME='ssxrver'
BUILD_DIR=${BUILD_DIR:-build}
BUILD_TYPE=${BUILD_TYPE:-Release}
#默认编译类型是Release,如果想改变编译类型请将Release改为Debug

mkdir -p $BUILD_DIR/$BUILD_TYPE \
    && cd $BUILD_DIR/$BUILD_TYPE \
    && cmake \
            -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
            $SOURCE_DIR \
    && make $*
