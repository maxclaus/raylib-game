# raylib-game

This is Falling World, a simple 2D game written in C using raylib library.

The game is available to play online at [itch.io](https://maxclaus.itch.io/falling-world).

## Running

### Desktop

```
cmake -S . -B build/desktop -DPLATFORM=Desktop -DCMAKE_BUILD_TYPE=Debug
cmake --build build/desktop
./game
```

### Web

Building the game for web requires:

- [emscripten](https://emscripten.org/docs/getting_started/downloads.html)

```
cmake -S . -B build/web -DPLATFORM=Web "-DCMAKE_TOOLCHAIN_FILE=../emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" -DCMAKE_BUILD_TYPE=Debug
cmake --build build/web
python -m http.server 8080 -d ./build/web
# or: emrun build/web/game.html
```

**Release**

Build the web version in release mode with `./scripts/release-web.sh` and upload the zip file to itch.io.

## Assets

- **Textures**: Assets used for hero and tile for textures are from [platformer/metroidvania asset pack](https://o-lobster.itch.io/platformmetroidvania-pixel-art-asset-pack) (by [O_Lobster](https://o-lobster.itch.io/)). The flag asset I created it using [piskelapp](https://www.piskelapp.com).
- **Sounds**: Sounds generated with [rFXGen](https://raylibtech.itch.io/rfxgen).

## Credits

- Thanks [Michael](https://github.com/vimichael) for his basic intro to raylib project in C++ project - [vimichael/intro-to-raylib](https://github.com/vimichael/intro-to-raylib), which I used as a bootstrap for this game.
- Thanks [O_Lobster](https://o-lobster.itch.io/) for providing incredible assets for 2D games.

## TODO

- Add a different kind of tile which resets the ground moving velocity.
- Add Address Sanitizer or Valgrind checking.
- Add a server to record game data.
