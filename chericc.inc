#!/usr/bin/env bash

source "config.inc"

DIR_CHERISDK_BIN="$DIR_CHERISDK/bin"

export CC="$DIR_CHERISDK_BIN/mips4-unknown-freebsd-clang"
export CXX="$DIR_CHERISDK_BIN/mips4-unknown-freebsd-clang++"
export CFLAGS="-g -integrated-as"
export LDFLAGS="-Wl,-melf64btsmip_fbsd -Wl,-g"

