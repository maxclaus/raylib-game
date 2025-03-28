#!/bin/sh

cmake -S . -B build/web -DPLATFORM=Web "-DCMAKE_TOOLCHAIN_FILE=../emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" -DCMAKE_BUILD_TYPE=Release
cmake --build build/web

echo ""
echo "Compressing output"

src_dir=build/web
dst_dir=build/web-release
zip_file=build/web-release.zip

rm -rf $dst_dir
mkdir -p $dst_dir
cp $src_dir/game.html $dst_dir/index.html
cp $src_dir/game.wasm $dst_dir/game.wasm
cp $src_dir/game.js $dst_dir/game.js
cp $src_dir/game.data $dst_dir/game.data
# cp $src_dir/game.mem $dst_dir/game.mem

zip $zip_file $dst_dir/*

echo ""
echo "File $zip_file has been generaged."
echo "Next, you need to upload it to itch.io."
