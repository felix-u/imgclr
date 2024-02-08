typedef struct {
    u32 width;
    u32 height;
    u16 *data;
} Farbfeld_Image;

static error farbfeld_read_from_memory(Str8 mem, Farbfeld_Image *out) {
    if (mem.len < 16) {
        return err("invalid farbfeld image: format header incomplete");
    }
    
    Str8 magic_value = str8_range(mem, 0, 8);
    if (!str8_eql(magic_value, str8("farbfeld"))) return errf(
        "invalid farbfeld image: expected format header magic value " 
            "'%.*s' but found '%.*s'",
        str8_fmt(str8("farbfeld")), str8_fmt(magic_value)
    );

    Str8 width_bytes = str8_range(mem, 8, 12);
    out->width = 
        ((u32)width_bytes.ptr[0] << 24) |
        ((u32)width_bytes.ptr[1] << 16) |
        ((u32)width_bytes.ptr[2] <<  8) |
        ((u32)width_bytes.ptr[3]);

    Str8 height_bytes = str8_range(mem, 12, 16);
    out->height = 
        ((u32)height_bytes.ptr[0] << 24) |
        ((u32)height_bytes.ptr[1] << 16) |
        ((u32)height_bytes.ptr[2] <<  8) |
        ((u32)height_bytes.ptr[3]);

    usize correct_image_size = 16 + (out->width * out->height * 8);
    if (correct_image_size != mem.len) return errf(
        "invalid farbfeld image: expected %zu bytes for %dx%d image, "
            "but found %zu bytes",
        correct_image_size, out->width, out->height, mem.len
    );

    out->data = (u16 *)(mem.ptr + 16);
    return 0;
}
