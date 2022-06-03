# imgclr

`imgclr` modifies images to fit a given colour palette.

Hubble example                                    | NASA example
:------------------------------------------------:|:--------------------------------------------:
![Hubble-original](examples/hubble1/original.jpg) | ![NASA-original](examples/nasa1/original.jpg)
![Hubble-original](examples/hubble1/convert1.jpg) | ![NASA-original](examples/nasa1/convert1.jpg)
![Hubble-original](examples/hubble1/convert2.jpg) | ![NASA-original](examples/nasa1/convert2.jpg)


### Features
- [x] Change palette of images
- [ ] ~~Read JPG and PNG as well as PPM~~ (Use imagemagick! See **Usage**)
- [ ] Use dithering to generate smoother results
- [ ] Allow the inversion of image brightness levels (convert dark images to
      light and vice versa)


### Usage

Edit the `inputPalette` at the top of the source code, just below the line `// EDIT PALETTE HERE`. The palette must
have exactly 18 colours in the default configuration, but you can modify the `paletteLen` variable a bit further down
to change this. The number of colours must match this variable exactly!

When ready, compile `imgclr` by running `make` in the project directory.

`imgclr` exclusively operates on the PPM image format. If you have `imagemagick` installed, you can process your JPGs
or PNGs by running `convert input.jpg input.ppm`, then running `imgclr`, and finally converting back to your preferred
format.

`./imgclr [-d] [-i input.ppm] [-o output.ppm] [-qs]`

**The `-i` and `-o` arguments are mandatory.**

`-d`: Output debug information. It is recommended to redirect output to a file, since several lines will be outputted per
pixel written, leading to raw text output in the hundreds of megabytes.

`-q`: "Quiet" - do **not** output basic information and progress bar.

`-s`: "Slow" - do **not** optimise by reusing the previous match if the current pixel is identical to the previous
	  pixel. This option exists purely for benchmarking and is not recommended.
