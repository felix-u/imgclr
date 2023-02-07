const std = @import("std");
const c = @cImport({
    @cDefine("ARGS_IMPLEMENTATION", "");
    @cDefine("ARGS_BINARY_NAME", "imgclr");
    @cDefine("ARGS_BINARY_VERSION", "0.2-dev");
    @cInclude("args.h");
    @cDefine("CLR_IMPLEMENATION", "");
    @cInclude("clr.h");
    @cDefine("DITHER_IMPLEMENTATION", "");
    @cInclude("dither.h");
    @cDefine("STBI_ONLY_JPEG", "");
    @cDefine("STBI_ONLY_PNG", "");
    @cDefine("STBI_ONLY_BMP", "");
    @cDefine("STBI_ONLY_PNM", "");
    @cDefine("STBI_IMAGE_IMPLEMENTATION", "");
    @cDefine("STBI_IMAGE_WRITE_IMPLEMENTATION", "");
    @cDefine("STBI_FAILURE_USERMSG", "");
    @cInclude("../libs/stb_image-v2.27/stb_image.h");
    @cInclude("../libs/stb_image_write-v1.16/stb_image_write.h");
});

pub fn main() !void {
    std.debug.print("All your {s} are belong to us.\n", .{"codebase"});
}

