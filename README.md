# imgclr

`imgclr` modifies images to fit a given colour palette.

![Banner image](examples/planet-volumes/planet-volumes-dither.jpg)

Dual-licensed under [GPL-3.0](./LICENSE-GPL3) or [MIT](./LICENSE-MIT). To get `imgclr`,
[download the latest release](https://github.com/felix-u/imgclr/releases) or follow the
[compilation instructions](#building).


### Summary

`imgclr`:

- Quantises images, changing their palette
- Supports JPG, PNG, and other formats
- Supports dithering
- Can invert image brightness while preserving colour 
    (converting dark images to light and vice versa)


### Examples

Input                                                | Result (dithered)
:---------------------------------------------------:|:---------------------------------------------------------------------:
![Original image](examples/jacek-dylag/original.jpg) | ![Processed image (dithered)](examples/jacek-dylag/output-dither.jpg)

#### Dithering

[Dithering](https://en.wikipedia.org/wiki/Dither) can increase the perceived
colour fidelity of a limited palette.

Input                                                | Result (simple): each pixel either black or white                               | Result (dithered): each pixel either black or white
:---------------------------------------------------:|:-------------------------------------------------------------------------------:|:-------------------------------------------------------------------------:
![Original image](examples/jacek-dylag/original.jpg) | ![Processed image (not dithered)](examples/jacek-dylag/monochrome-nodither.jpg) | ![Processed image (dithered)](examples/jacek-dylag/monochrome-dither.jpg)

With dithering disabled, `imgclr` naively
[quantises](https://en.wikipedia.org/wiki/Quantization_(image_processing)) the
image. Disable dithering by passing `--dither none`:
```sh
imgclr input.jpg output.jpg --palette 000 fff f00 0f0 00f --dither none
```
Input                                                | Result
:---------------------------------------------------:|:---------------------------------------------------------------------------:
![Original image](examples/jacek-dylag/original.jpg) | ![Processed image (not dithered)](examples/jacek-dylag/output-nodither.jpg)

Dithering is enabled by default (specifically, the [Floyd-Steinberg
algorithm](https://en.wikipedia.org/wiki/Floyd%E2%80%93Steinberg_dithering)).
Disabling dithering or using other algorithms may yield very different results.

#### Dithering algorithms

* **Floyd-Steinberg** (`--dither floyd-steinberg`)
    - Floyd-Steinberg dithering is very common due to its balance of quality
      and speed.
   ![Floyd-Steinberg](examples/algorithms/floyd-steinberg.jpg)

* **Atkinson** (`--dither atkinson`)
    - Developed for Apple in the 90s, Atkinson dithering propagates only some
      of the quantisation error, resulting in a higher-constrast look.
   ![Atkinson](examples/algorithms/atkinson.jpg)

* **Jarvis-Judice-Ninke** (`--dither jjn`)
    - Optimised for quality; relatively slow.
   ![Jarvis-Judice-Ninke](examples/algorithms/jjn.jpg)

* **Burkes** (`--dither burkes`)
    - A faster approximation of the previous algorithm.
   ![Burkes](examples/algorithms/burkes.jpg)

* **Sierra Lite** (`--dither sierra-lite`)
    - A slightly faster approximation of the Floyd-Steinberg algorithm.
   ![Sierra Lite](examples/algorithms/sierra-lite.jpg)

* Dithering disabled (`--dither none`)
  ![Dithering disabled](examples/algorithms/none.jpg)

#### Inverting brightness

The `--invert` flag inverts luminance whilst preserving hue and saturation
before quantisation. An example with the
[tokyonight](https://github.com/folke/tokyonight.nvim) colour scheme:

Input                                                   | Processed normally (no dithering)                       | Processed after luma inversion (no dithering)
:------------------------------------------------------:|:-------------------------------------------------------:|:--------------------------------------------------------------------------:
![Original image](examples/milad-fakurian/original.jpg) | ![Processed image](examples/milad-fakurian/convert.jpg) | ![Processed image with inversion](examples/milad-fakurian/convert-swap.jpg)


### Usage
```
imgclr <input file> <output file> <palette...> [options]

Options:
      --dither <algorithm>
        Specify dithering algorithm - one of:
            'floyd-steinberg' (default), 'none', 'atkinson', 'jjn',
            'burkes', 'sierra-lite'
      --invert
        Invert the image's luminance
      --palette <hex>...
        Specify palette - at least two (2) space-separated hex colours
  -h, --help
        Print this help and exit
      --version
        Print version information and exit
```


### Building

Using a C99 compiler, build `src/main.c` with optimisations. For example:
```sh
cc src/main.c -O3 -s -o ./imgclr
```


### Licence

imgclr is licensed under the terms of the MIT License, or alternatively under the terms of the General Public License
(GPL) Version 3. You may use imgclr according to either of these licences as is most appropriate for your project on a
case-by-case basis.

The terms of each licence can be found in the root directory of the imgclr source repository:

- MIT Licence: [LICENSE-MIT](./LICENSE-MIT)
- GPL3 Licence: [LICENSE-GPL3](./LICENSE-GPL3)

`SPDX-License-Identifier: MIT OR GPL-3.0-or-later`
