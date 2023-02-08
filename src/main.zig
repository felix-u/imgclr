const std = @import("std");
const c = @cImport({
    @cDefine("ARGS_IMPLEMENTATION", "");
    @cDefine("ARGS_BINARY_NAME", "\"imgclr\"");
    @cDefine("ARGS_BINARY_VERSION", "\"0.2-dev\"");
    @cInclude("args.h");
    @cDefine("CLR_IMPLEMENATION", "");
    @cInclude("clr.h");
    // Issues with translate-c, but I was going to rewrite this anyway.
    // @cDefine("DITHER_IMPLEMENTATION", "");
    // @cInclude("dither.h");
    @cDefine("STBI_ONLY_JPEG", "");
    @cDefine("STBI_ONLY_PNG", "");
    @cDefine("STBI_ONLY_BMP", "");
    @cDefine("STBI_ONLY_PNM", "");
    @cDefine("STBI_IMAGE_IMPLEMENTATION", "");
    @cDefine("STBI_IMAGE_WRITE_IMPLEMENTATION", "");
    @cDefine("STBI_FAILURE_USERMSG", "");
    @cInclude("stb_image-v2.27/stb_image.h");
    @cInclude("stb_image_write-v1.16/stb_image_write.h");
});

pub fn main() !void {

    var dither_flag = c.args_Flag {
        .name_short = 'd',
        .name_long = "dither",
        .help_text = "specify dithering algorithm, or 'none' to disable. Default\n" ++
                     "is 'floyd-steinberg'. Other options are: 'atkinson', 'jjn',\n" ++
                     "'burkes', and 'sierra-lite'",
        .required = false,
        .is_present = false,
        .opts = null,
        .opts_num = 0,
        .type = c.ARGS_MULTI_OPT,
        .expects = c.ARGS_EXPECTS_STRING,
    };
    _ = dither_flag;

    std.debug.print("All your {s} are belong to us.\n", .{"codebase"});
}

