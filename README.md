# imgclr

imgclr is an image colouriser written in C. It can be used to modify images to
fit a given colour palette.

## Features
- [x] Change palette of images
- [ ] Read JPG and PNG as well as PPM (might be better to just tie into ImageMagick to convert these formats to PPM and back again)
- [ ] Use dithering to generate smoother results
- [ ] Allow the inversion of image brightness levels (convert dark images to
      light and vice versa)

### Usage

`./imgclr -i input.ppm -o output.ppm`

The `-d` flag gives information helpful for debugging. Make sure you redirect to a log file, since several lines of information are generated per pixel written.
