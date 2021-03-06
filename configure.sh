#!/usr/bin/env bash

source chericc.inc

if [ "$1" == "-d" ]; then
    echo "Enabling debug..."
    JAMVM_DEBUG="--enable-trace"
fi

echo "Configuring GNU Classpath..."
pushd "$DIR_CLASSPATH" > /dev/null
    try_to_run ./configure \
      --prefix="$DIR_TARGET" \
      --host=mips4-unknown-freebsd \
      --with-sysroot="$DIR_CHERISDK/sysroot" \
      --enable-default-preferences-peer=file \
      --disable-plugin \
      --disable-gtk-peer \
      --disable-gconf-peer \
      --disable-qt-peer \
      --without-x \
      --disable-gjdoc \
      --disable-examples 
popd > /dev/null

echo "Configuring JamVM..."
pushd "$DIR_JAMVM" > /dev/null
    try_to_run ./configure \
      --prefix="$DIR_TARGET" \
      --host=mips4-unknown-freebsd \
      --with-sysroot="$DIR_CHERISDK/sysroot" \
      $JAMVM_DEBUG \
      --with-classpath-install-dir="$DIR_TARGET"
popd > /dev/null
