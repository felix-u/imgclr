#![allow(clippy::needless_range_loop)]
use clap::{Arg, Command};
use colored::*;
use color_processing::Color as ClrpColor;
use image::{GenericImageView, Rgb, RgbImage, Rgba, GenericImage, DynamicImage};
use indicatif::{ProgressBar, ProgressStyle};
use std::path::Path;

mod dither;

fn main() -> std::io::Result<()> {

    // parse command-line arguments
    let args = Command::new("imgclr")
        .about("Image colouriser")
        .args(&[
            Arg::new("dithering algorithm")
                .short('d')
                .long("algorithm")
                .required(false)
                .takes_value(true)
                .help("Specify dithering algorithm (case-insensitive). Valid options are \"floyd-steinberg\" \
                       (default), \"atkinson\", \"jjn\", \"burkes\", and \"sierra-lite\""),
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

    // get dithering algorithm, with the default being Floyd-Steinberg if not user-specified
    let mut algorithm: dither::Algorithm = dither::FLOYD_STEINBERG;
    // FIXME: this produces very odd, slightly dithered results
    if args.is_present("disable dithering") {
        algorithm = dither::NONE;
    }
    else if args.is_present("dithering algorithm") {
        let algorithm_argument: &str = &args.value_of("dithering algorithm").unwrap().to_ascii_lowercase();
        match algorithm_argument {
            "atkinson" =>           { algorithm = dither::ATKINSON; }
            "floyd-steinberg" =>    { algorithm = dither::FLOYD_STEINBERG; }
            "jjn" =>                { algorithm = dither::JARVIS_JUDICE_NINKE; }
            "burkes" =>             { algorithm = dither::BURKES; }
            "sierra-lite" =>        { algorithm = dither::SIERRA_LITE }
            &_ => { /* default */
                eprintln!("{} {} {}",
                    String::from("Error:").red().bold(),
                    String::from("no such dithering algorithm:"),
                    algorithm_argument.italic());
                std::process::exit(64/*USAGE*/);
            }
        }
    }

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
        eprintln!("{} {} {}",
            String::from("Error:").red().bold(),
            String::from("could not find file"),
            input_file.italic());
        std::process::exit(66/*NOINPUT*/);
    }

    // open input image
    let mut img_in = image::open(input_file)
                           .unwrap_or_else(|_| panic!("\n\n{} {}\n{}\n\n",
                            String::from("Error:").red().bold(),
                            String::from("could not open file as image."),
                            "Caught:"));

    let (width, height) = img_in.dimensions();
    if args.is_present("swap luma") { swap_luma(&mut img_in); }
    let mut img_buf: image::ImageBuffer<Rgb<u8>, Vec<u8>> = img_in.to_rgb8();
    let mut img_out = RgbImage::new(width, height);

    // progress bar
    println!("{}", "Converting image...".green().bold());
    let conversion_bar = ProgressBar::new(height as u64);
    conversion_bar.set_style(ProgressStyle::default_bar()
            .template("{eta:.cyan.bold} [{bar:27}] {percent}%  {msg:.blue.bold}")
            .progress_chars("=> "));

    // establish edge-case control loops (when not all error can be diffused because some of the
    // neighbouring pixels targeted by the dithering algorithm are out of bounds)
    let mut x_bound_left: u32 = 0;
    let mut x_bound_right: u32 = width;
    let mut y_bound_bottom: u32 = height;
    for (x_offset, y_offset, _) in algorithm.error {
        if *x_offset < x_bound_left as i32 {
            x_bound_left = (x_bound_left as i32 - *x_offset) as u32;
        }
        if (width as i32 - x_offset) < x_bound_right as i32 {
            x_bound_right = (width as i32 - *x_offset) as u32;
        }
        if (height as i32 - y_offset) < y_bound_bottom as i32 {
            y_bound_bottom = (height as i32 - *y_offset) as u32;
        }
    }


    // conversion
    let mut quant_error: [i16; 3] = [0, 0, 0];
    let mut prev_pixel: [u8; 3] = [0, 0, 0];

    for y in 0..y_bound_bottom {
        for x in 0..x_bound_left {
            match_from_palette(&mut img_buf, &x, &y, &palette, &mut img_out, &mut quant_error, &mut prev_pixel);
            for (x_offset, y_offset, error_amount) in algorithm.error {
                if (x as i32) > (-1 - *x_offset) {
                    dither::put_error(
                        &mut img_buf[(((x as i32) + *x_offset) as u32, ((y as i32) + *y_offset) as u32)],
                        &quant_error,
                        error_amount);
                }
            }
        }
        conversion_bar.inc(1);
    }

    for y in 0..y_bound_bottom {
        for x in x_bound_left..x_bound_right {
            match_from_palette(&mut img_buf, &x, &y, &palette, &mut img_out, &mut quant_error, &mut prev_pixel);
            for (x_offset, y_offset, error_amount) in algorithm.error {
                dither::put_error(
                    &mut img_buf[(((x as i32) + *x_offset) as u32, ((y as i32) + *y_offset) as u32)],
                    &quant_error,
                    error_amount);
            }
        }
        conversion_bar.inc(1);
    }

    for y in 0..y_bound_bottom {
        for x in x_bound_right..width {
            match_from_palette(&mut img_buf, &x, &y, &palette, &mut img_out, &mut quant_error, &mut prev_pixel);
            for (x_offset, y_offset, error_amount) in algorithm.error {
                if (x as i32) < ((width as i32)  - *x_offset) {
                    dither::put_error(
                        &mut img_buf[(((x as i32) + *x_offset) as u32, ((y as i32) + *y_offset) as u32)],
                        &quant_error,
                        error_amount);
                }
            }
        }
        conversion_bar.inc(1);
    }

    for y in y_bound_bottom..height {
        for x in 0..x_bound_left {
            match_from_palette(&mut img_buf, &x, &y, &palette, &mut img_out, &mut quant_error, &mut prev_pixel);
            for (x_offset, y_offset, error_amount) in algorithm.error {
                if (x as i32) > (-1 - *x_offset) &&
                   (y as i32) < ((height as i32) - *y_offset)
                {
                    dither::put_error(
                        &mut img_buf[(((x as i32) + *x_offset) as u32, ((y as i32) + *y_offset) as u32)],
                        &quant_error,
                        error_amount);
                }
            }
        }
        conversion_bar.inc(1);
    }

    for y in y_bound_bottom..height {
        for x in x_bound_left..x_bound_right {
            match_from_palette(&mut img_buf, &x, &y, &palette, &mut img_out, &mut quant_error, &mut prev_pixel);
            for (x_offset, y_offset, error_amount) in algorithm.error {
                if (y as i32) < ((height as i32) - *y_offset) {
                    dither::put_error(
                        &mut img_buf[(((x as i32) + *x_offset) as u32, ((y as i32) + *y_offset) as u32)],
                        &quant_error,
                        error_amount);
                }
            }
        }
        conversion_bar.inc(1);
    }

    for y in y_bound_bottom..height {
        for x in x_bound_right..width {
            match_from_palette(&mut img_buf, &x, &y, &palette, &mut img_out, &mut quant_error, &mut prev_pixel);
            for (x_offset, y_offset, error_amount) in algorithm.error {
                if (x as i32) < ((width as i32)  - *x_offset) &&
                   (y as i32) < ((height as i32) - *y_offset)
                {
                    dither::put_error(
                        &mut img_buf[(((x as i32) + *x_offset) as u32, ((y as i32) + *y_offset) as u32)],
                        &quant_error,
                        error_amount);
                }
            }
        }
        conversion_bar.inc(1);
    }

    conversion_bar.finish_with_message("Done!");


    // save image to output path
    match img_out.save(output_file) {
        Ok(()) => {},
        Err(e) => {
            eprintln!("Couldn't save output. Caught error: {}", e);
        }
    }

    println!("{} {}{}{} {} {}",
        String::from("Wrote").bold(),
        width.to_string().bold(),
        String::from("x").bold(),
        height.to_string().bold(),
        String::from("pixels to").bold(),
        output_file.italic().bold());
    Ok(())
}


fn match_from_palette(
    img_buf: &mut image::ImageBuffer<Rgb<u8>, Vec<u8>>,
    x: &u32,
    y: &u32,
    palette: &Vec<ClrpColor>,
    img_out: &mut image::ImageBuffer<Rgb<u8>, Vec<u8>>,
    quant_error: &mut[i16; 3],
    prev_pixel: &mut[u8; 3],
) {
    // get current pixel
    let this_r = img_buf[(*x, *y)][0];
    let this_g = img_buf[(*x, *y)][1];
    let this_b = img_buf[(*x, *y)][2];

    // if current pixel isn't identical to previous pixel, recompute match, quantisation error, etc.
    let mut best: &ClrpColor = &ClrpColor::new_rgb(0, 0, 0);
    if *prev_pixel != [this_r, this_g, this_b] {
        let mut best_match: usize = 0;

        // calculate best match from palette
        let mut min_diff: u16 = 999;
        for i in 0..Vec::len(palette) {
            let abs_diffs: [u16; 3] = [
                this_r.abs_diff(palette[i].red).into(),
                this_g.abs_diff(palette[i].green).into(),
                this_b.abs_diff(palette[i].blue).into(),
            ];
            let diff_total: u16 = abs_diffs[0] + abs_diffs[1] + abs_diffs[2];
            if diff_total < min_diff {
                min_diff = diff_total;
                best_match = i;
            }
        }

        best = &palette[best_match];
        *prev_pixel = [this_r, this_g, this_b];

        // dithering
        *quant_error = [
            this_r as i16 - best.red as i16,
            this_g as i16 - best.green as i16,
            this_b as i16 - best.blue as i16,
        ];

    }

    // write pixel
    img_out.put_pixel(*x, *y, Rgb([best.red, best.green, best.blue]));
}


fn swap_luma(some_img: &mut DynamicImage) {
    println!("{}", "Inverting image...".green().bold());
    for (x, y, pixel) in some_img.clone().pixels() {
        let this_pix = ClrpColor::new_rgb(pixel[0], pixel[1], pixel[2])
                        .invert_luminescence();
        some_img.put_pixel(x, y,
            Rgba([this_pix.red, this_pix.green, this_pix.blue, 255]));
    }
}
