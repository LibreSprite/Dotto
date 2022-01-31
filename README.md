# LibreSprite Dotto!

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
| Metal Hardware acceleration    | Planned       | 0.5     | Needed for OSX/iOS                              |
| x86/64 Linux                   | OK            | 0.5     |                                                 |
| x86/64 Windows                 | Planned       | 0.5     |                                                 |
| x86_64 MacOS                   | Planned       | 1.0     | Needs Metal backend first                       |
| Raspberry Pi 4 support         | Planned       | 1.0     | Needs OpenGL changes?                           |
| Undo / Redo                    | OK            | 0.5     |                                                 |
| Copy + Paste                   | Planned       | 0.5     |                                                 |
| Resize Sprite                  | Planned       | 0.5     |                                                 |
| Crop Sprite                    | Planned       | 0.5     |                                                 |
| Pencil tool                    | Basic         | 0.5     |                                                 |
| Pencil shapes/sizes/blending   | Planned       | 0.5     |                                                 |
| Line smoothing                 | Planned       | 1.0     |                                                 |
| Eraser tool                    | Planned       | 0.5     |                                                 |
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
| Layer alpha                    | Internal only | 0.5     |                                                 |
| Image Filter support           | OK            | 0.5     |                                                 |
| Drop Shadow filter             | OK            | 0.5     |                                                 |
| Blur Filter                    | Planned       | 0.5     |                                                 |
| Mirror/Flip image Filter       | Planned       | 0.5     |                                                 |
| Load Dotto format              | Planned       | 0.5     |                                                 |
| Load PNG, BMP, GIF, JPEG       | OK            | 0.5     | Using `libpng` and `SDL2_image`                 |
| Load LBM, PCX, PNM, SVG        | OK            | 0.5     | Using `SDL2_image`                              |
| Load TGA, TIFF, WEBP, XCF      | OK            | 0.5     | Using `SDL2_image`                              |
| Load XPM, XV                   | OK            | 0.5     | Using `SDL2_image`                              |
| Save PNG                       | OK            | 0.5     | Using `libpng`                                  |
| Save JPEG                      | OK            | 0.5     | Using `SDL2_image`                              |
| Save Dotto format              | Planned       | 0.5     |                                                 |
| Palette editor                 | Planned       | 0.5     |                                                 |
| Online Palette browser         | Planned       | 1.0     |                                                 |
| Online Resource/Script browser | Planned       | 1.0     |                                                 |
| eXPerience theme               | In Progress   | 0.5     | Windows XP-inspired skin                        |
| Futuretro theme                | In Progress   | 0.5     | Dark pixelart skin                              |
| Touchscreen-specific theme     | Planned       | 2.0     |                                                 |
| Non-destructive Layer Filters  | Planned       | 3.0     |                                                 |
| Commandline interface          | Planned       | 3.0     |                                                 |
| Animation support              | Planned       | 3.0     |                                                 |
| Android support                | Planned       | 4.0     |                                                 |
| iOS support                    | Planned       | 4.0     |                                                 |

## Compilation

### Arch Linux

Install dependencies:

`pacman -S make sdl2 sdl2_image lua v8-r `

Checkout project:

`git clone https://github.com/LibreSprite/Dotto`

Compile:

```sh
cd Dotto
make
```
