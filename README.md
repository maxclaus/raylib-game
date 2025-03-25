# raylib-game

Simple 2D game written in C using raylib library.

## Running

### Desktop

```
cmake -S . -B build/desktop -DPLATFORM=Desktop
cmake --build build/desktop
./game
```

### Web

Building the game for web requires:

- [emscripten](https://emscripten.org/docs/getting_started/downloads.html)

```
cmake -S . -B build/web -DPLATFORM=Web "-DCMAKE_TOOLCHAIN_FILE=../emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"
cmake --build build/web
python -m http.server 8080 -d ./build/web
# or: emrun build/web/game.html
```

## Assets

Assets used on this project come from [platformer/metroidvania asset pack](https://o-lobster.itch.io/platformmetroidvania-pixel-art-asset-pack) (by [O_Lobster](https://o-lobster.itch.io/)).

## TODO

- Change levels to have y position set based on the bottom of the screen instead of the top of the screen.
- Add instructions to setup assets for development.
- Add Address Sanitizer or Valgrind checking.
- Add a server to record game data.

## Credits

- Thanks [Michael](https://github.com/vimichael) for his basic intro to raylib project in C++ project - [vimichael/intro-to-raylib](https://github.com/vimichael/intro-to-raylib), which I used as a bootstrap for this game.
- Thanks [O_Lobster](https://o-lobster.itch.io/) for providing incredible assets for 2D games.
