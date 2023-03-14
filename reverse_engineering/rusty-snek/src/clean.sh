#!/bin/bash

# Compile with MUSL for better GLIBC compatibility.
docker run --rm -it -v "$(pwd)":/home/rust/src ekidd/rust-musl-builder cargo clean
