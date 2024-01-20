typedef enum Args_Kind {
    args_kind_bool = 0,
    args_kind_single_pos,
    args_kind_multi_pos,
} Args_Kind;

typedef struct Args_Flag {
    Str8 name;
    Args_Kind kind;
    
    bool is_present;
    Str8 single_pos;
    Slice_Str8 multi_pos;
} Args_Flag;

typedef Slice(Args_Flag *) Args_Flags;

typedef struct Args_Desc {
    Args_Kind exe_kind;
    Args_Flags flags;

    Str8 single_pos;
    Slice_Str8 multi_pos;
} Args_Desc;

typedef struct Args { char **argv; int argc; } Args;

static error args_parse(Arena *arena, int argc, char **argv, Args_Desc *desc) {
    if (argc == 0) return 1;
    if (argc == 1) switch (desc->exe_kind) {
        case args_kind_bool: return 0;
        default: return 1;
    }

    if (desc->exe_kind == args_kind_multi_pos) try (
        arena_alloc(arena, (argc - 1) * sizeof(Str8), &desc->multi_pos.ptr)
    );

    Slice(Str8) args = { 0 };
    try (arena_alloc(arena, (argc - 1) * sizeof(Str8), &args.ptr));
    for (int i = 1; i < argc; i += 1) {
        slice_push(args, str8_from_cstr(argv[i]));
    }

    Args_Flags *flags = &desc->flags;
    for (usize i = 0; i < flags->len; i += 1) {
        if (flags->ptr[i]->kind != args_kind_multi_pos) continue;
        try (arena_alloc(
            arena, 
            (argc - 1) * sizeof(Str8), 
            &flags->ptr[i]->multi_pos.ptr
        ));
    }

    Str8 arg = args.ptr[0];
    for (usize i = 0; i < args.len; i += 1, arg = args.ptr[i]) {
        if (arg.len == 1 || arg.ptr[0] != '-') {
            switch (desc->exe_kind) {
                case args_kind_bool: {
                    return errf(
                        "unexpected positional argument '%.*s'", 
                        str8_fmt(arg)
                    );
                } break;
                case args_kind_single_pos: {
                    if (desc->single_pos.len != 0 || 
                        desc->single_pos.ptr != 0
                    ) {
                        return errf(
                            "unexpected positional argument '%.*s'", 
                            str8_fmt(arg)
                        );
                    }
                    desc->single_pos = arg;
                } break;
                case args_kind_multi_pos: {
                    slice_push(desc->multi_pos, arg);
                } break;
            }
            continue;
        }

        Args_Flag *flag = 0;
        for (usize j = 0; j < flags->len; j += 1) {
            bool single_dash = (arg.ptr[0] == '-') && 
                str8_eql(str8_range(arg, 1, arg.len), flags->ptr[j]->name);
            bool double_dash = arg.len > 2 && 
                str8_eql(str8_range(arg, 0, 2), str8("--")) &&
                str8_eql(str8_range(arg, 2, arg.len), flags->ptr[j]->name);
            if (!single_dash && !double_dash) continue;

            flag = flags->ptr[j];
            flag->is_present = true;
            break;
        }
        if (flag == 0) return errf("invalid flag '%.*s'", str8_fmt(arg));

        switch (flag->kind) {
            case args_kind_bool: break;
            case args_kind_single_pos: {
                if (i + 1 == args.len || args.ptr[i + 1].ptr[0] == '-') {
                    return errf(
                        "expected positional argument after '%.*s'", 
                        str8_fmt(arg)
                    );
                }
                if (flag->single_pos.len != 0 || flag->single_pos.ptr != 0) {
                    return errf(
                        "unexpected positional argument '%.*s'", 
                        str8_fmt(arg)
                    );
                }
                flag->single_pos = args.ptr[++i];
            } break;
            case args_kind_multi_pos: {
                for (arg = args.ptr[++i]; i < args.len; arg = args.ptr[++i]) {
                    if (arg.ptr[0] == '-') {
                        i -= 1;
                        break;
                    }
                    slice_push(flag->multi_pos, arg);
                }
            } break;
        }
    }

    return 0;
}
