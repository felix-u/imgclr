# imgclr

`imgclr` modifies images to fit a given colour palette.

![Banner image](examples/planet-volumes/planet-volumes-dither.jpg)

Dual-licensed under [GPL-3.0](./LICENSE-GPL3) or [MIT](./LICENSE-MIT).

### Features
- [x] Change palette of images
- [x] Support JPG and PNG (at least)
- [x] Use dithering to generate smoother results
- [x] Allow the inversion of image brightness levels (convert dark images to light and vice versa)


### Building

First, download and install the latest *master version* of [the zig compiler](https://ziglang.org/download/). Then,
compile `imgclr` by running `git submodule update --init` and then `make` in the project directory. The compiled binary
will be copied to the repo's root directory.


### Usage

```
USAGE:
    imgclr [OPTIONS] --input <input file> --output <output file> --palette <palette>...

OPTIONS:
    -h, --help
            Display this help information and exit.

    -i, --input <FILE>
            Supply input file.

    -o, --output <FILE>
            Supply output location.

    -d, --dither <STR>
            Specify dithering algorithm, or "none" to disable (default is "floyd-steinberg").

    -p, --palette <STR>...
            Supply palette as whitespace-separated colours.

    -s, --swap
            Invert image brightness, preserving hue and saturation.
```
Note that the `-i`/`--input` and `-o`/`--output` arguments are **required**.

`imgclr` uses the [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) and
[lodepng](https://github.com/lvandeve/lodepng), allowing it to support JPG, PNG, and BMP.

Palette colours are supplied as hex values with the `-p` or `--palette` flag. For example, you could represent perfect
red as `#ff0000` or `f00`. Depending on your shell and whether you choose to include hash symbols, they may have to be
quoted.
* `#ff0000`
* `f00`

Here's what an `imglcr` command using black, white, red, green, and blue might look like:
```sh
imgclr -i input.jpg -o output.jpg -p 000 fff f00 0f0 00f
```

Input                                                | Result (dithered)
:---------------------------------------------------:|:---------------------------------------------------------------------:
![Original image](examples/jacek-dylag/original.jpg) | ![Processed image (dithered)](examples/jacek-dylag/output-dither.jpg)

#### Dithering

You'll notice that the output looks suspiciously similar to the input, and perhaps slightly grainy. This is because
`imgclr` uses [Floyd-Steinberg dithering](https://en.wikipedia.org/wiki/Floyd%E2%80%93Steinberg_dithering) to smoothen
the output, making it seem as if there is more colour fidelity than there really is. With dithering, a dark purple
colour in the input image may be approximated using your supplied blue, red, and black, even if you specified no
purple. This works due to the same effect that makes a red and white striped shirt appear pink from a distance.

Input                                                | Result (simple): each pixel either black or white                               | Result (dithered): each pixel either black or white
:---------------------------------------------------:|:-------------------------------------------------------------------------------:|:-------------------------------------------------------------------------:
![Original image](examples/jacek-dylag/original.jpg) | ![Processed image (not dithered)](examples/jacek-dylag/monochrome-nodither.jpg) | ![Processed image (dithered)](examples/jacek-dylag/monochrome-dither.jpg)

With dithering disabled, `imgclr` simply goes through each pixel, choosing the closest match from your input palette.
Let's retry our example in colour, this time disabling dithering by passing `--dither/-d none`:

```sh
imgclr -i input.jpg -o output.jpg -p 000 fff f00 0f0 00f --dither none
```

Input                                                | Result (simple)
:---------------------------------------------------:|:---------------------------------------------------------------------------:
![Original image](examples/jacek-dylag/original.jpg) | ![Processed image (not dithered)](examples/jacek-dylag/output-nodither.jpg)

Dithering is enabled by default due to its great improvement of results and low impact on speed (50% or less).  For
more abstract or cartoonish images, disabling dithering or using the "Atkinson" algorithm will generally yield
better-looking results. You could experiment with the other algorithms to find your favourite.

#### Dithering algorithms

Multiple dithering algorithms are supported by `imgclr`. The best way to decide on an algorithm is to just *try them!*
But if that's not enough for you, below are descriptions of each algorithm, including example images. The "baseline"
speed comparisons compares to running imgclr with dithering disabled and are approximations.

* **Floyd-Steinberg** (`-d floyd-steinberg`) - 25% slower than baseline
    - Floyd-Steinberg dithering is common in a variety of use cases, popular for its balance of quality and speed. It is
   the default for this reason.
   ![Floyd-Steinberg](examples/algorithms/floyd-steinberg.jpg)

* **Atkinson** (`-d atkinson`) - 35% slower than baseline
    - Developed for Apple in the 90s, Atkinson dithering propagates only some of the quantisation error, resulting in a
   more contrasty look which better suits simpler or more abstract images.
   ![Atkinson](examples/algorithms/atkinson.jpg)

* **Jarvis-Judice-Ninke** (`-d jjn`) - 80% slower than baseline
    - Optimised for quality, and the slowest in the list.
   ![Jarvis-Judice-Ninke](examples/algorithms/jjn.jpg)

* **Burkes** (`-d burkes`) - 40% slower than baseline
    - Essentially a faster version of Jarvis-Judice-Ninke which achieves approximately the same look.
   ![Burkes](examples/algorithms/burkes.jpg)

* **Sierra Lite** (`-d sierra-lite`) - 20% slower than baseline
    - Faster approximation of Floyd-Steinberg dithering.
   ![Sierra Lite](examples/algorithms/sierra-lite.jpg)

* **DITHERING DISABLED** (`-d none`) - baseline
  ![Dithering disabled](examples/algorithms/none.jpg)


#### Inverting brightness

The `-s` or `--swap` flag inverts luminance whilst preserving hue and saturation. For example, perfect grey will remain
the same, black will become white, white will become black, and dark green will become light green. This inverted
version of your input image is what will be processed to generate the output image. Here's an example using the
[tokyonight](https://github.com/folke/tokyonight.nvim) colour scheme:

Input                                                   | Processed normally (no dithering)                       | Processed after luma inversion (no dithering)
:------------------------------------------------------:|:-------------------------------------------------------:|:--------------------------------------------------------------------------:
![Original image](examples/milad-fakurian/original.jpg) | ![Processed image](examples/milad-fakurian/convert.jpg) | ![Processed image with inversion](examples/milad-fakurian/convert-swap.jpg)

Notice that the bottom-most line keeps its colour. Usually, red would invert to green, and purple to yellow.

#### Using Xresources

You may wish to automatically pass in your Xresources theme colours. The `xres.sh` script is included for this very
purpose: at the top of the script, just set the path to your Xresources file (by default `~/.Xresources`) and the
path to the `imgclr` binary (by default `./imgclr`), and run the script, passing in the
same arguments you would use with `imgclr`. The script simply *parses* a file in the Xresources format, which means
it'll also work on Wayland, in the TTY, or on any operating system with a shell that can run it.

### Licence

imgclr is licensed under the terms of the MIT License, or alternatively under the terms of the General Public License
(GPL) Version 3. You may use imgclr according to either of these licences as is most appropriate for your project on a
case-by-case basis.

The terms of each licence can be found in the root directory of the imgclr source repository:

- MIT Licence: [LICENSE-MIT](./LICENSE-MIT)
- GPL3 Licence: [LICENSE-GPL3](./LICENSE-GPL3)

`SPDX-License-Identifier: MIT OR GPL-3.0-or-later`

