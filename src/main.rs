use clap::{Arg, Command};
use color_processing::Color as ClrpColor;
use exitcode;
use image::{ImageFormat, GenericImageView, Rgb, RgbImage, Rgba, GenericImage,
            DynamicImage};
use std::{fs, env};
use std::path::Path;
use rand::{thread_rng, Rng};
use rand::distributions::Alphanumeric;


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
                .help("Disable dithering"),
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
    
    // copy input file to tempfile
    let rand_alphanum: String = thread_rng()
        .sample_iter(&Alphanumeric)
        .take(8)
        .map(char::from)
        .collect();
    let temp_dir = env::temp_dir().into_os_string().into_string().unwrap();
    let ext = ImageFormat::extensions_str(
        ImageFormat::from_path(input_file).unwrap())[0];
    let temp_file = temp_dir + "/" + rand_alphanum.as_str() + "." + ext;
    println!("Using tempfile at {}", temp_file);
    fs::copy(input_file, &temp_file)?;
    
    // open input image, using tempfile
    let img_in = image::open(&temp_file)
                        .expect("Could not open image. Caught error");
    let (width, height) = img_in.dimensions();
    // open output image
    let mut img_out = RgbImage::new(width, height);
    

    // conversion
    for (x, y, pixel) in img_in.pixels() {

        // get current pixel
        let this_r: i16;
        let this_g: i16;
        let this_b: i16;
        if args.is_present("swap luma")  {
            let this_pix = ClrpColor::new_rgb(pixel[0], pixel[1], pixel[2])
                            .invert_luminescence();
            this_r = this_pix.red as i16;
            this_g = this_pix.green as i16;
            this_b = this_pix.blue as i16;
        }
        else {
            // pixel is an array. index 0 is R, 1 is G, 2 is B, and 3 is alpha
            this_r = pixel[0] as i16;
            this_g = pixel[1] as i16;
            this_b = pixel[2] as i16;
        }
    
        // find best match
        let mut best_match = 0;
        let mut min_diff: u16 = 999;
        for i in 0..Vec::len(&palette) {
            let comp_r: u16 = this_r.abs_diff(palette[i].red as i16).into();
            let comp_g: u16 = this_g.abs_diff(palette[i].green as i16).into();
            let comp_b: u16 = this_b.abs_diff(palette[i].blue as i16).into();
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
        
        // dithering - see https://en.wikipedia.org/wiki/Floyd-Steinberg_dithering
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
                    [that_r, that_g, that_b], img_in.clone());
            }
            
            // 2
            if x > 0 && y < (height - 1) {
                let that_pix = img_in.get_pixel(x-1, y+1);
                let that_r = that_pix[0];
                let that_g = that_pix[1];
                let that_b = that_pix[2];
                put_quantised(x-1, y+1, quant_error, 3,
                    [that_r, that_g, that_b], img_in.clone());
            }
            
            // 3
            if y < (height - 1) {
                let that_pix = img_in.get_pixel(x, y+1);
                let that_r = that_pix[0];
                let that_g = that_pix[1];
                let that_b = that_pix[2];
                put_quantised(x, y+1, quant_error, 5,
                    [that_r, that_g, that_b], img_in.clone());
            }
            
            // 4
            if x < (width - 1) && y < (height - 1) {
                let that_pix = img_in.get_pixel(x+1, y+1);
                let that_r = that_pix[0];
                let that_g = that_pix[1];
                let that_b = that_pix[2];
                put_quantised(x+1, y+1, quant_error, 1,
                    [that_r, that_g, that_b], img_in.clone());
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
                             channels: [u8; 3], mut some_img: DynamicImage) {
    
    let new_r = (channels[0] as i16 + error[0] * numerator / 16) as u8;
    let new_g = (channels[1] as i16 + error[1] * numerator / 16) as u8;
    let new_b = (channels[2] as i16 + error[2] * numerator / 16) as u8;
    some_img.put_pixel(loc_x, loc_y, Rgba([
        new_r, new_g, new_b, 255  
    ]))
}
