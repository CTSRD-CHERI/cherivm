#!/usr/bin/env bash

source "config.inc"

DIR_CHERISDK_BIN="$DIR_CHERISDK/bin"

export CC="$DIR_CHERISDK_BIN/cheri-unknown-freebsd-clang"
export CXX="$DIR_CHERISDK_BIN/cheri-unknown-freebsd-clang++"
export CFLAGS="-g -integrated-as --sysroot=$DIR_CHERISYS -B$DIR_CHERISDK -msoft-float -all-static -G0 -Qunused-arguments"
export CCASFLAGS="$CFLAGS -no-integrated-as" # don't use -integrated-as with .S files
export LDFLAGS="-Wl,-melf64btsmip_fbsd -Wl,-g"
export OBJCOPY="$DIR_CHERISDK_BIN/objcopy"

export DIR_JAMVM_INCLUDE="$DIR_JAMVM/src"
export DIR_CHERIJNI_INSTALL="$DIR_TARGET/lib/classpath"

