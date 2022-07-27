use image::Rgb;


pub struct Algorithm {
    // (x_offset, y_offset, error multiplier)
    error: &'static[(i8, i8, f32)],
}

// https://gazs.github.io/canvas-atkinson-dither
pub const ATKINSON: Algorithm = Algorithm {
    error: &[
                       /* CURRENT PIXEL */ (1, 0, 1.0/8.0), (2, 0, 1.0/8.0),
        (-1, 1, 1.0/8.0), (0, 1, 1.0/8.0), (1, 1, 1.0/8.0),
                          (0, 2, 1.0/8.0),
    ]
};
// https://en.wikipedia.org/wiki/Floyd–Steinberg_dithering
pub const FLOYD_STEINBERG: Algorithm = Algorithm {
    error: &[
                         /* CURRENT PIXEL */ (1, 0, 7.0/16.0),
        (-1, 1, 3.0/16.0), (0, 1, 5.0/16.0), (1, 1, 1.0/16.0),
    ]
};


#[derive(Debug)]
pub struct QuantError(Vec<(i16, i16, i16)>);
pub fn init_error(algorithm: Algorithm) -> QuantError {
    QuantError(vec![(0, 0, 0); algorithm.error.len()])
}


pub fn put_quantised(error: &[i16; 3], numerator: i16, channels: [u8; 3], loc: &mut Rgb<u8>) {
    let mut new_r = channels[0] as i16 + error[0] * numerator / 16;
    let mut new_g = channels[1] as i16 + error[1] * numerator / 16;
    let mut new_b = channels[2] as i16 + error[2] * numerator / 16;
    flatten(&mut new_r);
    flatten(&mut new_g);
    flatten(&mut new_b);
    *loc = Rgb([new_r as u8, new_g as u8, new_b as u8]);
}


// fixes any overflows caused by dithering computation making channel values
// smaller than 0 or greater than 255
fn flatten(n: &mut i16) {
    if *n > 255 {
        *n = 255;
    }
    if *n < 0 {
        *n = 0;
    }
}
