use image::Rgb;


pub struct Algorithm {
    // (x_offset, y_offset, error multiplier)
    pub error: &'static[(i32, i32, f32)],
}


pub const NONE: Algorithm = Algorithm {
    error: &[(0, 0, 0.0)],
};
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
// https://en.wikipedia.org/wiki/Error_diffusion
pub const JARVIS_JUDICE_NINKE: Algorithm = Algorithm {
    error: &[
                                            /* CURRENT PIXEL */ (1, 0, 7.0/48.0), (2, 0, 5.0/48.0),
        (-2, 1, 3.0/48.0), (-1, 1, 5.0/48.0), (0, 1, 7.0/48.0), (1, 1, 5.0/48.0), (2, 1, 3.0/48.0),
        (-2, 2, 1.0/48.0), (-1, 2, 3.0/48.0), (0, 2, 5.0/48.0), (1, 2, 3.0/48.0), (2, 2, 1.0/48.0),
    ]
};


pub fn put_error(img_loc: &mut Rgb<u8>, quant_error: &[i16; 3], error_amount: &f32) {
    let mut new_r = img_loc[0] as i16 + (quant_error[0] as f32 * error_amount) as i16;
    let mut new_g = img_loc[1] as i16 + (quant_error[1] as f32 * error_amount) as i16;
    let mut new_b = img_loc[2] as i16 + (quant_error[2] as f32 * error_amount) as i16;
    flatten(&mut new_r);
    flatten(&mut new_g);
    flatten(&mut new_b);
    *img_loc = Rgb([new_r as u8, new_g as u8, new_b as u8]);

    // fixes any overflows caused by dithering computation making channel values smaller than 0 or greater than 255
    fn flatten(n: &mut i16) {
        if *n > 255 { *n = 255; }
        if *n < 0 { *n = 0; }
    }
}
