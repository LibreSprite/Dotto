# LibreSprite Dotto! 

## [![Linux x86_64 CI](https://github.com/LibreSprite/Dotto/actions/workflows/c-cpp-linux-x86_64.yml/badge.svg?branch=development)](https://github.com/LibreSprite/Dotto/actions/workflows/c-cpp-linux-x86_64.yml) [![Windows i686 CI](https://github.com/LibreSprite/Dotto/actions/workflows/c-cpp-windows-i686.yml/badge.svg)](https://github.com/LibreSprite/Dotto/actions/workflows/c-cpp-windows-i686.yml) [![MacOS x86_64 CI](https://github.com/LibreSprite/Dotto/actions/workflows/c-cpp-macos-x86_64.yml/badge.svg)](https://github.com/LibreSprite/Dotto/actions/workflows/c-cpp-macos-x86_64.yml)

## Funding / Contributing

For the time being, this project is fuelled purely by the power of the **stars**.
As in, github stars.
We use them to light and heat the crates we keep our codegnomes in. As of this
writing, the repo has 21 stars to its 390 commits, so we still have plenty of
gnomes coding in cold, dark crates, which is a problem in the winter.
Since they don't want to freeze their little gnomefingers off, the exchange
rate is currently set to *5 commits per star*.

So, if you want to contribute to the project but don't know how to code, feel
free to give the repo a star. *Think of the codegnomes.*

## Stability Notice

This project is still in early stages of development and problems are to be expected.
Instead of filing an issue here, it is faster and more productive to reach out
over Discord/Matrix.

## Version Roadmap

| Number | Notes                                                |
| ---    | ---                                                  |
| 0.5    | Preview release with basic image editing support     |
| 1.0    | First stable release focused on static image editing |
| 2.0    | Polish based on feedback from previous releases      |
| 3.0    | Animation Support                                    |
| 4.0    | Additional platforms support (Android / iOS)         |

## Feature Roadmap

| Feature                        | Status        | Version | Notes                                           |
| ---------------                | --------      | ---     | -----                                           |
| Themes support                 | OK            | 0.5     |                                                 |
| Scripting support              | OK            | 0.5     |                                                 |
| Internationalization           | OK            | 0.5     |                                                 |
| OpenGL Hardware acceleration   | OK            | 1.0     |                                                 |
| x86/64 Linux                   | OK            | 0.5     |                                                 |
| x86/64 Windows                 | OK            | 0.5     |                                                 |
| x86_64 MacOS                   | OK            | 0.5     |                                                 |
| Nintendo 3DS                   | OK            | 1.0     | Will probably require specific theme            |
| Raspberry Pi 4 support         | OK            | 1.0     |                                                 |
| Undo / Redo                    | OK            | 0.5     |                                                 |
| Copy + Paste                   | Planned       | 0.5     | Waiting for selection support                   |
| Resize Sprite                  | Planned       | 0.5     |                                                 |
| Crop Sprite                    | Planned       | 0.5     | Waiting for selection support                   |
| Pencil tool                    | OK            | 0.5     |                                                 |
| Pencil shapes/sizes            | OK            | 0.5     |                                                 |
| Pencil blending modes          | Planned       | 1.0     |                                                 |
| Line smoothing                 | Basic         | 1.0     |                                                 |
| Eraser tool                    | OK            | 0.5     |                                                 |
| Text tool                      | Planned       | 1.0     |                                                 |
| Selection                      | Internal only | 0.5     | Selections implemented but inaccessible to user |
| Rectangular Selection tool     | Planned       | 0.5     |                                                 |
| Wand Selection tool            | Planned       | 0.5     |                                                 |
| Dither tool                    | Planned       | 2.0     |                                                 |
| Fill Bucket                    | OK            | 0.5     |                                                 |
| Fill Bucket blending           | Planned       | 0.5     |                                                 |
| Gradient tool                  | Planned       | 2.0     |                                                 |
| Layers support                 | Partial       | 0.5     |                                                 |
| Layer groups                   | Planned       | 2.0     |                                                 |
| Layer masks                    | Planned       | 2.0     |                                                 |
| Layer blending                 | Planned       | 0.5     |                                                 |
| Layer alpha                    | OK            | 0.5     |                                                 |
| Image Filter support           | OK            | 0.5     |                                                 |
| Drop Shadow filter             | OK            | 0.5     |                                                 |
| Solarize                       | Planned       | 2.0     |                                                 |
| Sobel edge-detection           | Planned       | 2.0     |                                                 |
| Mirror image                   | OK            | 0.5     |                                                 |
| Flip image                     | OK            | 0.5     |                                                 |
| Curves adjustment              | Planned       | 1.0     |                                                 |
| Salt and Pepper filter         | Planned       | 2.0     |                                                 |
| 2D Fast Fourier Transform      | Planned       | 2.0     |                                                 |
| Load Dotto format              | Planned       | 0.5     |                                                 |
| Load PNG, BMP, GIF, JPEG       | OK            | 0.5     | Using `libpng` and `SDL2_image`                 |
| Load LBM, PCX, PNM, SVG        | OK            | 0.5     | Using `SDL2_image`                              |
| Load TGA, TIFF, WEBP, XCF      | OK            | 0.5     | Using `SDL2_image`                              |
| Load XPM, XV                   | OK            | 0.5     | Using `SDL2_image`                              |
| Load QOI                       | OK            | 0.5     | Using `qoi.h`                                   |
| Save QOI                       | OK            | 0.5     | Using `qoi.h`                                   |
| Save PNG                       | OK            | 0.5     | Using `libpng`                                  |
| Save JPEG                      | OK            | 0.5     | Using `SDL2_image`                              |
| Save Dotto format              | Planned       | 0.5     |                                                 |
| Palette editor                 | Planned       | 0.5     |                                                 |
| Online Palette browser         | Planned       | 1.0     |                                                 |
| Online Resource/Script browser | Planned       | 1.0     |                                                 |
| eXPerience theme               | In Progress   | 0.5     | Windows XP-inspired skin                        |
| Futuretro theme                | In Progress   | 0.5     | Dark pixelart skin                              |
| Photoshop theme                | In Progress   | 0.5     | Photoshop inspired skin                         |
| Touchscreen-specific theme     | Planned       | 2.0     |                                                 |
| Non-destructive Layer Filters  | Planned       | 3.0     |                                                 |
| Commandline interface          | Planned       | 3.0     |                                                 |
| Animation support              | Planned       | 3.0     |                                                 |
| Android support                | Planned       | 4.0     |                                                 |

## Compilation

### Linux

Install dependencies:

- Arch-based distros: `pacman -S make sdl2 sdl2_image lua v8-r lcms2`

- Debian-based distros: `sudo apt-get install libsdl2-dev libsdl2-image-dev liblcms2-dev liblua5.3-dev libfreetype6-dev libnode-dev`

Checkout project:

`git clone https://github.com/LibreSprite/Dotto`

Compile:

```sh
cd Dotto
make
```

If you are using GCC version 8 or less, you will probably get errors like this one:
`Folder.cpp:(.text+0x274): undefined reference to std::filesystem::__cxx11::path::_M_split_cmpts()'`
To fix this, use:

```sh
make OLDGCC=true
```

If Dotto compiles fine but you are getting a crash on startup, it might be due
to outdated/buggy libraries. After a `make clean`, try compiling again with:

```sh
make DEBIANELDERLY=true
```

If you are trying to run Dotto under VirtualBox and all you see is a glitched window, you need to *disable* 3D acceleration in the VM settings. Dotto requires functioning OpenGL 3.3 to work.

If you want to make a debug build for development, use:

```sh
make DEBUG=true
```

### Windows

Install [MSYS2](https://msys2.org).

Initial setup: Open the MSYS2 shell and run:

```sh
pacman -Syu
pacman -S base-devel git mingw-w64-i686-gcc mingw-w64-i686-openssl mingw-w64-i686-v8  mingw-w64-i686-SDL2  mingw-w64-i686-SDL2_image  mingw-w64-i686-lcms2  mingw-w64-i686-lua  mingw-w64-i686-freetype mingw-w64-i686-glew
git clone https://github.com/LibreSprite/Dotto.git
```

Compiling:

Run the Mingw32 shell (C:/msys64/mingw32.exe).

```sh
cd Dotto
make DEBUG=false
./dotto.exe
```

### MacOS

Initial setup:

```sh
brew install sdl2 sdl2_image lcms2 lua freetype v8 openssl
```

Compiling:

```sh
PKG_CONFIG_PATH="/usr/local/opt/openssl/lib/pkgconfig" make DEBUG=false
chmod +x dotto
./dotto
```

### Nintendo 3DS

- Install devkitpro
- Install dependencies: devkitARM, citro3d, 3ds-sdl, 3ds-sdl_image, 3ds-zlib, 3ds-lz4, 3ds-libpng
- Compile: `make -j 4 -f Makefile.3ds`
- Upload the resulting Dotto.elf.3dsx to your 3DS.
- Create a folder called `dotto` in the root of your SD card.
- Copy the `data` folder into the `dotto` folder (`/dotto/data/`).
