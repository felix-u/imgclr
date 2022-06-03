use clap::{Arg,Command};
use std::path::Path;
use image::{GenericImageView, GenericImage, ImageBuffer, Pixel, Rgb, RgbImage};
use exitcode;

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
            Arg::new("verbose")
                .short('v')
                .long("verbose")
                .required(false)
                .takes_value(false)
                .help("Print additional information"),
        ]).get_matches();

    let input_file = args.value_of("input file").unwrap();
    let output_file = args.value_of("output file").unwrap();

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

    // copy input file as-is to output path
    // fs::copy(input_file, output_file)?;
    // open output image
    // let mut img_out = image::open(output_file).expect("Could not open output image. Caught error");
    let mut img_out = RgbImage::new(width, height);

    // process image
    for (x, y, pixel) in img_in.pixels() {

        // pixel is an array. index 0 is R, 1 is G, 2 is B, and 3 is alpha
        img_out.put_pixel(x, y, Rgb([255, 255, 255]));

        // println!("{:?}", pixel);
        // Working placeholder - copy pixel-by-pixel with no changes
        // img_out.put_pixel(x, y,
        //                   pixel.map(|p| p))

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
