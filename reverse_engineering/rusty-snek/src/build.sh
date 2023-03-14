#!/bin/bash

# Compile with MUSL for better GLIBC compatibility.
docker run --rm -it -v "$(pwd)":/home/rust/src ekidd/rust-musl-builder cargo build --release

echo "Binary is located: $(find . -name rusty_snake)"
