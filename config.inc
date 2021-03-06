#!/usr/bin/env bash

DIR_ROOT=`pwd`
DIR_REALROOT=`realpath .`
DIR_SRC="$DIR_ROOT/src"
DIR_CLASSPATH="$DIR_SRC/classpath"
DIR_JAMVM="$DIR_SRC/jamvm"
DIR_TARGET="$DIR_ROOT/target"
DIR_CHERISDK="$DIR_ROOT/sdk"
DIR_CHERISDK_BIN="$DIR_CHERISDK/bin"

export JFLAG=-j`sysctl -n kern.smp.cpus`

mkdir -p "$DIR_TARGET"

try_to_run()
{
  $@ > ${DIR_ROOT}/error.log 2>&1
  if [ $? -ne 0 ] ; then
     echo $1 failed, see error.log for details
     exit 1
  fi
}

