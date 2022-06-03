use clap::{Arg,Command};
use std::path::Path;
use image::{GenericImageView, ImageBuffer, Pixel};
use exitcode;

fn main() {

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


    // // create output image
    // let mut img_out = ImageBuffer::new(width, height);
    // for (x, y, pixel) in img_in.pixels() {
    //     // palette conversion process somewhere in here lol
    //     // FIXME: Don't just copy with no changes lol
    //     img_out.put_pixel(x, y,
    //                       pixel.map(|p| p))
    // }

    // working placeholder: copy input to output path with no changes
    match img_in.save(output_file) {
        Ok(()) => {},
        Err(e) => {
            eprintln!("Could not save to output path. Caught error:\n{}", e);
        }
    }

}
