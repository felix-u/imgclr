use clap::{Arg, Command};
use color_processing::Color as ClrpColor;
use exitcode;
use image::{GenericImageView, Rgb, RgbImage};
use std::path::Path;

fn main() -> std::io::Result<()> {

    // parse command-line arguments
    let args = Command::new("imgclr")
        .about("Image colouriser")
        .args(&[
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
                .help("Supply palette as whitespace-separated hex values"),
            Arg::new("verbose")
                .short('v')
                .long("verbose")
                .required(false)
                .takes_value(false)
                .help("Print additional information"),
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

    // open the image
    let img_in = image::open(input_file)
                        .expect("Could not open image. Caught error");
    let (width, height) = img_in.dimensions();

    if args.is_present("verbose") {
        print!("Input file: {}\nOutput file: {}\n", input_file, output_file);
        println!("Of dimensions {} by {}", width, height);
    }

    // open output image
    let mut img_out = RgbImage::new(width, height);

    // process image
    for (x, y, pixel) in img_in.pixels() {

        // pixel is an array. index 0 is R, 1 is G, 2 is B, and 3 is alpha
        let this_r = pixel[0];
        let this_g = pixel[1];
        let this_b = pixel[2];

        // set up array for comparisons
        let mut best_match = 0;
        let mut min_diff: u16 = 999;
        for i in 0..Vec::len(&palette) {
            let comp_r: u16 = this_r.abs_diff(palette[i].red).into();
            let comp_g: u16 = this_g.abs_diff(palette[i].green).into();
            let comp_b: u16 = this_b.abs_diff(palette[i].blue).into();
            let diff_total: u16 = comp_r + comp_g + comp_b;
            if diff_total < min_diff {
                min_diff = diff_total;
                best_match = i;
            }
        }

        let clr_match = &palette[best_match];
        let best_r = clr_match.red;
        let best_g = clr_match.green;
        let best_b = clr_match.blue;
        img_out.put_pixel(x, y, Rgb([best_r, best_g, best_b]));

    }

    // save to output path
    match img_out.save(output_file) {
        Ok(()) => {},
        Err(e) => {
            eprintln!("Couldn't save output. Caught error: {}", e);
        }
    }

    Ok(())
}
