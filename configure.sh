#!/usr/bin/env bash

source chericc.inc

echo "Configuring GNU Classpath..."
pushd "$DIR_CLASSPATH" > /dev/null
    try_to_run ./configure \
      --prefix="$DIR_TARGET" \
      --target=mips4-unknown-freebsd \
      --host=x86_64-unknown-freebsd10.0 \
      --with-sysroot="$DIR_CHERISDK" \
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
      --with-sysroot="$DIR_CHERISDK" \
      --with-classpath-install-dir="$DIR_TARGET" \
      --enable-trace
popd > /dev/null
