#![allow(clippy::needless_range_loop)]
use clap::{Arg, Command};
use color_processing::Color as ClrpColor;
use image::{GenericImageView, Rgb, RgbImage, Rgba, GenericImage, DynamicImage};
use std::path::Path;

fn main() -> std::io::Result<()> {

    // parse command-line arguments
    let args = Command::new("imgclr")
        .about("Image colouriser")
        .args(&[
            Arg::new("disable dithering")
                .short('n')
                .long("no-dither")
                .required(false)
                .takes_value(false)
                .help("Disable Floyd-Steinberg dithering"),
            Arg::new("input file")
                .short('i')
                .long("input")
                .required(true)
                .takes_value(true)
                .help("Supply path to input file"),
            Arg::new("output file")
                .short('o')
                .long("output")
                .required(true)
                .takes_value(true)
                .help("Supply path to output file"),
            Arg::new("palette")
                .multiple_values(true)
                .short('p')
                .long("palette")
                .required(true)
                .takes_value(true)
                .help("Supply palette as whitespace-separated colours"),
            Arg::new("swap luma")
                .short('s')
                .long("swap")
                .required(false)
                .takes_value(false)
                .help("Invert image brightness, preserving hue and saturation"),
        ]).get_matches();

    let input_file = args.value_of("input file").unwrap();
    let output_file = args.value_of("output file").unwrap();

    // get palette
    let palette_input: Vec<_> = args.values_of("palette").unwrap().collect();

    // save palette to array in processed format
    let mut palette: Vec<ClrpColor> = Vec::new();
    for i in 0..Vec::len(&palette_input) {
        let str_clr = ClrpColor::new_string(palette_input[i]).unwrap();
        palette.push(str_clr);
    }

    // check that input file exists and error out if not
    if !Path::new(input_file).exists() {
        eprintln!("Error: could not find {}", input_file);
        std::process::exit(exitcode::NOINPUT);
    }

    // open input image
    let mut img_in = image::open(input_file)
                           .expect("Could not open image. Caught error");
    let (width, height) = img_in.dimensions();
    // open output image
    let mut img_out = RgbImage::new(width, height);

    if args.is_present("swap luma") {
        swap_luma(&mut img_in);
    }

    // conversion
    for x in 0..width {
        for y in 0..height {

            // get current pixel
            let pixel = img_in.get_pixel(x, y);
            let this_r = pixel[0] as i16;
            let this_g = pixel[1] as i16;
            let this_b = pixel[2] as i16;

            // find best match
            let mut best_match = 0;
            let mut min_diff: u16 = 999;
            for i in 0..Vec::len(&palette) {
                let comp_r: u16 = this_r.abs_diff(palette[i].red as i16);
                let comp_g: u16 = this_g.abs_diff(palette[i].green as i16);
                let comp_b: u16 = this_b.abs_diff(palette[i].blue as i16);
                let diff_total: u16 = comp_r + comp_g + comp_b;
                if diff_total < min_diff {
                    min_diff = diff_total;
                    best_match = i;
                }
            }
            let clr_match = &palette[best_match];
            let best_r = clr_match.red as i16;
            let best_g = clr_match.green as i16;
            let best_b = clr_match.blue as i16;

            // write pixel
            img_out.put_pixel(x, y, Rgb([best_r as u8, best_g as u8, best_b as u8]));

            // dithering
            // https://en.wikipedia.org/wiki/Floyd-Steinberg_dithering
            if !args.is_present("disable dithering") {

                let quant_error: [i16; 3] = [
                    this_r - best_r,
                    this_g - best_g,
                    this_b - best_b
                ];

                // operates on the following, where * is the current pixel
                //   * 1
                // 2 3 4

                // 1
                if x < (width - 1) {
                    let that_pix = img_in.get_pixel(x+1, y);
                    let that_r = that_pix[0];
                    let that_g = that_pix[1];
                    let that_b = that_pix[2];
                    put_quantised(x+1, y, quant_error, 7,
                        [that_r, that_g, that_b], &mut img_in);
                }

                // 2
                if x > 0 && y < (height - 1) {
                    let that_pix = img_in.get_pixel(x-1, y+1);
                    let that_r = that_pix[0];
                    let that_g = that_pix[1];
                    let that_b = that_pix[2];
                    put_quantised(x-1, y+1, quant_error, 3,
                        [that_r, that_g, that_b], &mut img_in);
                }

                // 3
                if y < (height - 1) {
                    let that_pix = img_in.get_pixel(x, y+1);
                    let that_r = that_pix[0];
                    let that_g = that_pix[1];
                    let that_b = that_pix[2];
                    put_quantised(x, y+1, quant_error, 5,
                        [that_r, that_g, that_b], &mut img_in);
                }

                // 4
                if x < (width - 1) && y < (height - 1) {
                    let that_pix = img_in.get_pixel(x+1, y+1);
                    let that_r = that_pix[0];
                    let that_g = that_pix[1];
                    let that_b = that_pix[2];
                    put_quantised(x+1, y+1, quant_error, 1,
                        [that_r, that_g, that_b], &mut img_in);
                }

            }
        }
    }

    // save image to output path
    match img_out.save(output_file) {
        Ok(()) => {},
        Err(e) => {
            eprintln!("Couldn't save output. Caught error: {}", e);
        }
    }

    // FIXME: uncomment when done implementing dithering!
    // fs::remove_file(temp_file)?;
    println!("Wrote image of size {}x{} to {}", width, height, output_file);
    Ok(())
}


fn put_quantised(loc_x: u32, loc_y: u32, error: [i16; 3], numerator: i16,
                             channels: [u8; 3], some_img: &mut DynamicImage) {
    let mut new_r = channels[0] as i16 + error[0] * numerator / 16;
    let mut new_g = channels[1] as i16 + error[1] * numerator / 16;
    let mut new_b = channels[2] as i16 + error[2] * numerator / 16;
    flatten(&mut new_r);
    flatten(&mut new_g);
    flatten(&mut new_b);
    some_img.put_pixel(loc_x, loc_y, Rgba([
        new_r as u8, new_g as u8, new_b as u8, 255
    ]));
}


// TODO: abstract the dithering computation, incorporating this overflow
//       checking into said abstraction directly
// fixes any overflows due to 
fn flatten(n: &mut i16) {
    if *n > 255 {
        *n = 255;
    }
    if *n < 0 {
        *n = 0;
    }
}    


fn swap_luma(some_img: &mut DynamicImage) {
    for (x, y, pixel) in some_img.clone().pixels() {
        let this_pix = ClrpColor::new_rgb(pixel[0], pixel[1], pixel[2])
                        .invert_luminescence();
        some_img.put_pixel(x, y, 
            Rgba([this_pix.red, this_pix.green, this_pix.blue, 255]));
    } 
}
