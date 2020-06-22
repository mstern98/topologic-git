#!/bin/bash -ex
set -ex
cd rustopologic/RustWrap/src
cp ../../../src/*.c .
mkdir ../include
cp  ../../../include/* ../include
bindgen ../../../include/topologic.h -o bindings.rs
cd ..
cargo build --verbose
