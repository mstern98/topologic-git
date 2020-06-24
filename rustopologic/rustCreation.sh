#!/bin/bash -ex
set -ex
cd rustopologic/RustTopologic/src
cp ../../../src/*.c .
mkdir -p ../include
cp  ../../../include/* ../include
bindgen ../../../include/topologic.h -o bindings.rs
cd ..
cp src/bindings.rs rustTests/
cp ../../libtopologic.a .
cargo build --verbose --release
