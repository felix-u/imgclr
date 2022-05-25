# imgclr

imgclr is an image colouriser written in C. It can be used to modify images to
fit a given colour palette.

### Features
- [x] Change palette of images
- [ ] Read JPG and PNG as well as PPM (might be better to just tie into ImageMagick to convert these formats to PPM and back again)
- [ ] Use dithering to generate smoother results
- [ ] Allow the inversion of image brightness levels (convert dark images to
      light and vice versa)

### Usage

To compile `imgclr`, run `make` in the project directory.

`./imgclr [-dq] [-p palette] [-i input.ppm] [-o output.ppm]`

**The `-i` and `-o` arguments are mandatory.**

`-d`: Output debug information. It is recommended to redirect output to a file, since several lines will be outputted per pixel written.

`-p file`: [NOT FUNCTIONAL] Read palette from file containing whitespace-separated hex values.

`-q`: Do not output basic information and progress bar ("quiet").
