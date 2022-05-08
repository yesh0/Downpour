# downpour

[![Linux AppImage Build](https://github.com/yesh0/downpour/actions/workflows/ci.yml/badge.svg?branch=v0.0.1)](https://github.com/yesh0/downpour/actions/workflows/ci.yml)

An tiny OOP project.

You may download pre-built binary files from [Releases](https://github.com/yesh0/downpour/releases).

## If you want to build it yourself...

I am not sure if this builds for Windows or MacOS so easily. (Well, obviously it does not in the case of GitHub Actions.)

### Linux

Install the following dependencies as well as their development headers: (They are actually [SFML dependencies](https://www.sfml-dev.org/tutorials/2.5/compile-with-cmake.php))

- freetype
- x11
- xrandr
- udev
- opengl
- flac
- ogg
- vorbis
- vorbisenc
- vorbisfile
- openal
- pthread

The library names may vary. If you are using Ubuntu, you might want to have a look at [the commands I use for GitHub Actions](./.github/workflows/ci.yml).

After that,

```sh
mkdir -p build
cd build
cmake -G "Unix Makefiles" ..
make -j4
```

If the above commands complete without errors, you may run the game with:

```sh
src/downpour
```

## Licenses of Used Works

### Code

- Box2D
  - Liquidfun
- SFML
- pugixml

### Media

- The bunny sprite:
  - From [bunnemoji](https://github.com/bunnegirl/bunnemoji), after-processed with GIMP to pixelize a bit.
- Rain drop sounds:
  - From [https://www.fesliyanstudios.com/royalty-free-sound-effects-download/water-drops-and-bloops-17](https://www.fesliyanstudios.com/royalty-free-sound-effects-download/water-drops-and-bloops-17)
- Rain fall (waterfall) sound:
  - From [https://freesound.org/people/straget/sounds/489073/](https://freesound.org/people/straget/sounds/489073/) by [straget](https://freesound.org/people/straget/) [CC-BY-4.0](https://creativecommons.org/licenses/by/4.0/).
- Font:
  - Press Start 2P (which is licensed under the [Open Font License](https://scripts.sil.org/cms/scripts/page.php?site_id=nrsi&id=OFL)).
  - And the Nimbus font family and the Source Sans / Serif family for making some of the images (I don't remember precisely which is for which though)

### Software

- The media files are constructed with:
  - [GIMP](https://www.gimp.org/): Drawing image files
  - [Tiled](https://github.com/mapeditor/tiled): Making tiled maps
  - [GDX Texture Packer](https://github.com/crashinvaders/gdx-texture-packer-gui/): Packing images
- Built with [CMake](https://cmake.org/)
- Debugging:
  - GDB
  - Valgrind

### Helpful StackOverflow Q / A

- How to configure VSCode clangd to work with CMake-based projects:
  - [Generating compile_commands.json](https://stackoverflow.com/questions/23960835/cmake-not-generating-compile-commands-json)