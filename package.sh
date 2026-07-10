# use after building wasm to the folder build/cmake-build-wasm/
pushd build/cmake-build-wasm/ && cp ./Project.html index.html && zip game0.3.zip ./Project.* ./index.html && popd