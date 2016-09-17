#!/bin/bash

rm -rf $LIBRARIES_ROOT
mkdir $LIBRARIES_ROOT

rm -rf $SHEENBIDI_ROOT
mkdir $SHEENBIDI_ROOT

git clone https://github.com/mta452/SheenBidi $SHEENBIDI_ROOT
make -C $SHEENBIDI_ROOT
mv $SHEENBIDI_ROOT/Release/*.a $LIBRARIES_ROOT/
