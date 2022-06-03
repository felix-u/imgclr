use clap::{Arg,Command};

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

    if args.is_present("verbose") {
        print!("Input file: {}\nOutput file: {}\n", input_file, output_file);
    }

}
