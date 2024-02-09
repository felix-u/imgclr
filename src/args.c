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
    struct { int beg_i; int end_i; } multi_pos;
} Args_Flag;

typedef Slice(Args_Flag *) Args_Flags;

typedef struct Args_Desc {
    Args_Kind exe_kind;
    Args_Flags flags;

    Str8 single_pos;
    struct { int beg_i; int end_i; } multi_pos;
} Args_Desc;

typedef struct Args { char **argv; int argc; } Args;

static error args_parse(int argc, char **argv, Args_Desc *desc) {
    if (argc == 0) return 1;
    if (argc == 1) switch (desc->exe_kind) {
        case args_kind_bool: return 0;
        default: return 1;
    }

    Args_Flags *flags = &desc->flags;
    Str8 arg = str8_from_cstr(argv[1]);
    for (int i = 1; i < argc; i += 1, arg = str8_from_cstr(argv[i])) {
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
                    if (desc->multi_pos.beg_i == 0) {
                        desc->multi_pos.beg_i = (int)i;
                        desc->multi_pos.end_i = (int)(i + 1);
                        continue;
                    }

                    if (desc->multi_pos.end_i < (int)i) return errf(
                        "unexpected positional argument '%.*s'; "
                            "positional arguments ended earlier",
                        str8_fmt(arg)
                    );
                    
                    desc->multi_pos.end_i += 1;
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
                if (i + 1 == argc || argv[i + 1][0] == '-') return errf(
                    "expected positional argument after '%.*s'", 
                    str8_fmt(arg)
                );
                if (flag->single_pos.len != 0 || flag->single_pos.ptr != 0) {
                    return errf(
                        "unexpected positional argument '%.*s'", 
                        str8_fmt(arg)
                    );
                }
                flag->single_pos = str8_from_cstr(argv[++i]);
            } break;
            case args_kind_multi_pos: {
                if (i + 1 == argc || argv[i + 1][0] == '-') return errf(
                    "expected positional argument after '%.*s'", 
                    str8_fmt(arg)
                );

                flag->multi_pos.beg_i = i + 1;
                flag->multi_pos.end_i = i + 2;

                for (
                    arg = str8_from_cstr(argv[++i]); 
                    i < argc; 
                    arg = str8_from_cstr(argv[++i])
                ) {
                    if (arg.ptr[0] == '-') {
                        i -= 1;
                        break;
                    }
                    flag->multi_pos.end_i = i + 1;
                }
            } break;
        }
    }

    return 0;
}
