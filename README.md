# imgclr

`imgclr` modifies images to fit a given colour palette.

<!-- <p> -->
<!-- 	<img alt="Hubble image - original" src="examples/hubble1/original.jpg" width="35%"/> -->
<!-- 	&nbsp; &nbsp; &nbsp; &nbsp; -->
<!-- 	<img alt="NASA image - original" src="examples/nasa1/original.jpg" width="35%"/> -->
<!-- 	<br> -->
<!-- 	<img alt="Hubble image - solarised palette" src="examples/hubble1/convert1.jpg" width="35%"/> -->
<!-- 	&nbsp; &nbsp; &nbsp; &nbsp; -->
<!-- 	<img alt="NASA image - solarised palette" src="examples/nasa1/convert1.jpg" width="35%"/> -->
<!-- 	<br> -->
<!-- 	<img alt="Hubble image - alternative palette" src="examples/hubble1/convert2.jpg" width="35%"/> -->
<!-- 	&nbsp; &nbsp; &nbsp; &nbsp; -->
<!-- 	<img alt="NASA image - alternative palette" src="examples/nasa1/convert2.jpg" width="35%"/> -->
<!-- </p> -->

Hubble example                                    | NASA example
:------------------------------------------------:|:--------------------------------------------:
![Hubble-original](examples/hubble1/original.jpg) | ![NASA-original](examples/nasa1/original.jpg)
![Hubble-original](examples/hubble1/convert1.jpg) | ![NASA-original](examples/nasa1/convert1.jpg)
![Hubble-original](examples/hubble1/convert2.jpg) | ![NASA-original](examples/nasa1/convert2.jpg)


### Features
- [x] Change palette of images
- [ ] Read JPG and PNG as well as PPM (might be better to just tie into ImageMagick to convert these formats to PPM and
      back again)
- [ ] Use dithering to generate smoother results
- [ ] Allow the inversion of image brightness levels (convert dark images to
      light and vice versa)


### Usage

Since support for reading the palette from stdin is not yet implemented, edit the `inputPalette` in the source code,
just below the line `// EDIT PALETTE HERE`. The palette must have exactly 18 colours in the default configuration,
but you can modify the `paletteLen` variable a bit further down to change this. The number of colours must match this
variable exactly!

When ready, compile `imgclr` by running `make` in the project directory.

`./imgclr [-d] [-i input.ppm] [-o output.ppm] [-p palette] [-qs]`

**The `-i` and `-o` arguments are mandatory.**

`-d`: Output debug information. It is recommended to redirect output to a file, since several lines will be outputted per
pixel written, leading to raw text output in the hundreds of megabytes.

`-p file`: [NOT FUNCTIONAL] Read palette from file containing whitespace-separated hex values.

`-q`: "Quiet" - do **not** output basic information and progress bar.

`-s`: "Slow" - do **not** optimise by reusing the previous match if the current pixel is identical to the previous
	  pixel. This option exists purely for benchmarking and is not recommended.
