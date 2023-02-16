const std = @import("std");

const print = std.debug.print;


const FlagExpectsWhat = enum {
    none,
    boolean,
    single_option,
    multiple_options,
};

const FlagExpectsType = enum {
    arg_generic,
    number,
    string,
    path,
};

pub const Flag = struct {
    name_short: ?u8 = null,
    name_long: ?[]const u8 = null,
    help_text: ?[]const u8 = null,
    arg_expects_what: FlagExpectsWhat = .none,
    arg_expects_type: ?FlagExpectsType = null,
    is_required: bool = false,
    is_present: bool = false,
    args: ?[][:0]const u8 = null,
};


pub fn process(argv: []const [:0]const u8) !void {
    for (argv) |arg| {
        print("{s}\n", .{arg});
    }
}
// #define ARGS_MISSING_FLAG_TEXT "option '--%s' is required"
// #define ARGS_MISSING_ARG_TEXT "option '--%s' requires an argument"
// #define ARGS_INVALID_FLAG_TEXT "invalid option"
// #define ARGS_MISSING_POSITIONAL_TEXT "expected %s"
// #define ARGS_USAGE_ERR_HELP_TEXT "Try '%s --%s' for more information."
// #define ARGS_HELP_FLAG_NAME_SHORT 'h'
// #define ARGS_HELP_FLAG_NAME_LONG "help"
// #define ARGS_HELP_FLAG_HELP_TEXT "display this help and exit"
// #define ARGS_VERSION_FLAG_NAME_SHORT false
// #define ARGS_VERSION_FLAG_NAME_LONG "version"
// #define ARGS_VERSION_FLAG_HELP_TEXT "output version information and exit"

// void args_helpHint(void) {
//     #ifndef ARGS_HELP_FLAG_DISABLED
//     printf(ARGS_USAGE_ERR_HELP_TEXT, ARGS_BINARY_NAME, ARGS_HELP_FLAG_NAME_LONG);
//     putchar('\n');
//     #endif // ARGS_HELP_FLAG_DISABLED
// }
//
//
// bool args_optionalFlagsPresent(const size_t flags_count, args_Flag *flags[]) {
//     for (size_t i = 0; i < flags_count; i++) {
//         if (!flags[i]->required && flags[i]->is_present) return true;
//     }
//     return false;
// }
//
//
// int args_process
// (int argc, char **argv, const char *usage_description, const size_t flags_count, args_Flag *flags[],
// size_t *positional_num, char **positional_args, const ARGS_FLAG_EXPECTS positional_expects,
// const ARGS_BINARY_POSITIONAL_TYPE positional_type, const size_t positional_cap)
// {
//     #ifdef ARGS_HELP_FLAG_DISABLED
//     (void) usage_description;
//     #endif // ARGS_HELP_FLAG_DISABLED
//
//     // All flags MUST have a long format, if not a short one.
//     // While we're looping over flags, if none are required, let's not mention mandatory options in help text later.
//     bool any_mandatory = false;
//     for (size_t i = 0; i < flags_count; i++) {
//         assert(flags[i]->name_long != NULL && "One flag has a NULL name_long");
//         if (flags[i]->required == true) any_mandatory = true;
//     }
//
//     #ifndef ARGS_HELP_FLAG_DISABLED
//     args_Flag *help_flag = args_byNameShort(ARGS_HELP_FLAG_NAME_SHORT, flags_count, flags);
//     bool help_implied = false;
//     #endif // ARGS_HELP_FLAG_DISABLED
//
//     // Immediately show help if binary was expecting positional arguments but got none.
//     #ifndef ARGS_HELP_FLAG_DISABLED
//     if ((positional_type == ARGS_POSITIONAL_SINGLE || positional_type == ARGS_POSITIONAL_MULTI) && argc == 1) {
//         if (help_flag != NULL) help_flag->is_present = true;
//         help_implied = true;
//     }
//     #endif // ARGS_HELP_FLAG_DISABLED
//
//     int skip = 0;
//     bool no_more_flags = false; // All arguments are positional after "--", as is convention.
//     bool is_positional[argc];
//     for (int i = 0; i < argc; i++) is_positional[i] = false;
//     *positional_num = 0;
//
//     for (int i = 1; i + skip < argc; i++) {
//         i += skip;
//         skip = 0;
//
//         char *arg = argv[i];
//         size_t arg_len = strlen(arg);
//
//         if ((arg[0] != '-' || no_more_flags) && (size_t)i <= positional_cap) {
//             (*positional_num)++;
//             is_positional[i] = true;
//             continue;
//         }
//
//         // @Note { There is no convention for dealing with arg = '-'. I skip it. }
//
//         // Arg starts with "--"
//         if (arg_len > 1 && arg[1] == '-') {
//
//             // Double dash ("--") means only positionals beyond this point!
//             if (arg_len == 2) {
//                 no_more_flags = true;
//                 continue;
//             }
//
//             // Arg is of form "--arg"
//             bool found_match = false;
//             for (size_t j = 0; j < flags_count; j++) {
//                 if (!strncasecmp(flags[j]->name_long, (arg + 2), (arg_len))) {
//                     flags[j]->is_present = true;
//                     if (flags[j]->type == ARGS_SINGLE_OPT && (i + 1) < argc &&
//                        (strlen(argv[i + 1]) >= 1 && argv[i + 1][0] != '-'))
//                     {
//                         flags[j]->opts = argv + i + 1;
//                         flags[j]->opts_num = 1;
//                         skip = 1;
//                     }
//                     else if (flags[j]->type == ARGS_MULTI_OPT) {
//                         size_t opts_num = 0;
//                         for (int k = i + 1; k < argc; k++) {
//                             if (strlen(argv[k]) > 1 && argv[k][0] == '-') break;
//                             opts_num++;
//                         }
//                         flags[j]->opts = argv + i + 1;
//                         flags[j]->opts_num = opts_num;
//                         skip = opts_num;
//                     }
//                     found_match = true;
//                     break;
//                 }
//             }
//             // Flag invalid
//             if (!found_match) {
//                 printf("%s: %s '%s'\n", ARGS_BINARY_NAME, ARGS_INVALID_FLAG_TEXT, argv[i]);
//                 #ifndef ARGS_HELP_FLAG_DISABLED
//                 printf(ARGS_USAGE_ERR_HELP_TEXT, ARGS_BINARY_NAME, ARGS_HELP_FLAG_NAME_LONG);
//                 putchar('\n');
//                 #endif // ARGS_HELP_FLAG_DISABLED
//                 return EX_USAGE;
//             }
//
//             continue;
//         }
//
//         // Arg is of form "-arg"
//
//         // Go up to last character
//         for (size_t j = 1; j < arg_len - 1; j++) {
//             bool found_match = false;
//             for (size_t k = 0; k < flags_count; k++) {
//                 if (arg[j] == flags[k]->name_short) {
//                     flags[k]->is_present = true;
//                     found_match= true;
//                     break;
//                 }
//             }
//             // Flag invalid
//             if (!found_match) {
//                 printf("%s: %s '%c' in '%s'\n", ARGS_BINARY_NAME, ARGS_INVALID_FLAG_TEXT, arg[j], arg);
//                 #ifndef ARGS_HELP_FLAG_DISABLED
//                 printf(ARGS_USAGE_ERR_HELP_TEXT, ARGS_BINARY_NAME, ARGS_HELP_FLAG_NAME_LONG);
//                 putchar('\n');
//                 #endif // ARGS_HELP_FLAG_DISABLED
//                 return EX_USAGE;
//             }
//         }
//         // Last character could have options supplied to it ("-arg opt opt" == "-a -r -g opt opt").
//         bool found_match = false;
//         for (size_t k = 0; k < flags_count; k++) {
//             if (arg[arg_len - 1] == flags[k]->name_short) {
//                 flags[k]->is_present = true;
//                 found_match = true;
//                 if (flags[k]->type == ARGS_SINGLE_OPT && (i + 1) < argc &&
//                    (strlen(argv[i + 1]) >= 1 && argv[i + 1][0] != '-'))
//                 {
//                     flags[k]->opts = argv + i + 1;
//                     flags[k]->opts_num = 1;
//                     skip = 1;
//                 }
//                 else if (flags[k]->type == ARGS_MULTI_OPT) {
//                     size_t opts_num = 0;
//                     for (int l = i + 1; l < argc; l++) {
//                         if (strlen(argv[l]) > 1 && argv[l][0] == '-') break;
//                         opts_num++;
//                     }
//                     flags[k]->opts = argv + i + 1;
//                     flags[k]->opts_num = opts_num;
//                     skip = opts_num;
//                 }
//             }
//         }
//         // Flag invalid
//         if (!found_match) {
//             if (arg_len > 2) {
//                 printf("%s: %s '%c' in '%s'\n", ARGS_BINARY_NAME, ARGS_INVALID_FLAG_TEXT, arg[arg_len - 1], arg);
//             }
//             else printf("%s: %s '%s'\n", ARGS_BINARY_NAME, ARGS_INVALID_FLAG_TEXT, arg);
//
//             #ifndef ARGS_HELP_FLAG_DISABLED
//             printf(ARGS_USAGE_ERR_HELP_TEXT, ARGS_BINARY_NAME, ARGS_HELP_FLAG_NAME_LONG);
//             putchar('\n');
//             #endif // ARGS_HELP_FLAG_DISABLED
//             return EX_USAGE;
//         }
//     }
//
//     // Fill array of positional args
//     size_t positional_counter = 0;
//     for (int i = 1; i < argc; i++) {
//         if (is_positional[i] == true && positional_counter < positional_cap) {
//             positional_args[positional_counter] = argv[i];
//             positional_counter++;
//         }
//     }
//
//     // Help text
//     #ifndef ARGS_HELP_FLAG_DISABLED
//     if (help_flag != NULL && help_flag->is_present == true) {
//         printf(ARGS_BINARY_NAME);
//         if (usage_description != NULL) printf(" - %s", usage_description);
//         #ifndef ARGS_VERSION_FLAG_DISABLED
//         printf(" (version %s)", ARGS_BINARY_VERSION);
//         #endif // ARGS_VERSION_FLAG_DISABLED
//
//         printf("\n\nUSAGE:\n");
//         printf("%s <OPTION>", ARGS_BINARY_NAME);
//         if (flags_count > 1) printf("...");
//         if (positional_expects != ARGS_EXPECTS_NONE) {
//             switch (positional_expects) {
//                 case ARGS_EXPECTS_NUM:
//                     printf(" <NUM>");
//                     break;
//                 case ARGS_EXPECTS_STRING:
//                     printf(" <STR>");
//                     break;
//                 case ARGS_EXPECTS_FILE:
//                     printf(" <FILE>");
//                     break;
//                 default:
//                     printf(" <ARG>");
//             }
//             if (positional_type == ARGS_POSITIONAL_MULTI) printf("...");
//         }
//         putchar('\n');
//         if (any_mandatory) printf("Options marked with '*' are mandatory.\n");
//
//         printf("\nOPTIONS:\n");
//         for (size_t i = 0; i < flags_count; i++) {
//             if (flags[i]->required == true) printf("* ");
//             else printf("  ");
//
//             if (flags[i]->name_short != false) printf("-%c, ", flags[i]->name_short);
//             else printf("    ");
//
//             if (flags[i]->name_long != NULL) printf("--%s", flags[i]->name_long);
//
//             if (flags[i]->expects != ARGS_EXPECTS_NONE) {
//                 switch (flags[i]->expects) {
//                     case ARGS_EXPECTS_NUM:
//                         printf(" <NUM>");
//                         break;
//                     case ARGS_EXPECTS_STRING:
//                         printf(" <STR>");
//                         break;
//                     case ARGS_EXPECTS_FILE:
//                         printf(" <FILE>");
//                         break;
//                     default:
//                         printf(" <ARG>");
//                 }
//                 if (flags[i]->type == ARGS_MULTI_OPT) printf("...");
//             }
//
//             putchar('\n');
//
//             if (flags[i]->help_text != NULL) {
//                 putchar('\t');
//                 size_t help_len = strlen(flags[i]->help_text);
//                 for (size_t j = 0; j < help_len; j++) {
//                     putchar(flags[i]->help_text[j]);
//                     if (flags[i]->help_text[j] == '\n') putchar('\t');
//                 }
//                 putchar('\n');
//             }
//         }
//
//         if (help_implied) return EX_USAGE;
//         return EXIT_SUCCESS;
//     }
//     #endif // ARGS_HELP_FLAG_DISABLED
//
//     // Version number
//     #ifndef ARGS_VERSION_FLAG_DISABLED
//     args_Flag *version_flag = args_byNameLong(ARGS_VERSION_FLAG_NAME_LONG, flags_count, flags);
//     if (version_flag != NULL && version_flag->is_present) {
//         printf("%s %s\n", ARGS_BINARY_NAME, ARGS_BINARY_VERSION);
//         return EXIT_SUCCESS;
//     }
//     #endif // ARGS_VERSION_FLAG_DISABLED
//
//     // Check mandatory flags
//     for (size_t i = 0; i < flags_count; i++) {
//         if (flags[i]->required && !flags[i]->is_present) {
//             printf("%s: ", ARGS_BINARY_NAME);
//             printf(ARGS_MISSING_FLAG_TEXT, flags[i]->name_long);
//             putchar('\n');
//             #ifndef ARGS_HELP_FLAG_DISABLED
//             printf(ARGS_USAGE_ERR_HELP_TEXT, ARGS_BINARY_NAME, ARGS_HELP_FLAG_NAME_LONG);
//             putchar('\n');
//             #endif // ARGS_HELP_FLAG_DISABLED
//             return EX_USAGE;
//         }
//         else if (flags[i]->is_present && (flags[i]->type == ARGS_SINGLE_OPT || flags[i]->type == ARGS_MULTI_OPT)
//                  && flags[i]->opts_num < 1)
//         {
//             printf("%s: ", ARGS_BINARY_NAME);
//             printf(ARGS_MISSING_ARG_TEXT, flags[i]->name_long);
//             putchar('\n');
//             #ifndef ARGS_HELP_FLAG_DISABLED
//             printf(ARGS_USAGE_ERR_HELP_TEXT, ARGS_BINARY_NAME, ARGS_HELP_FLAG_NAME_LONG);
//             putchar('\n');
//             #endif // ARGS_HELP_FLAG_DISABLED
//             return EX_USAGE;
//         }
//     }
//
//     // Exit if positional arguments were expected, but not received.
//     if ((positional_type == ARGS_POSITIONAL_SINGLE || positional_type == ARGS_POSITIONAL_MULTI)
//         && *positional_num == 0)
//     {
//         printf("%s: ", ARGS_BINARY_NAME);
//         switch (positional_expects) {
//             case ARGS_EXPECTS_NUM:
//                 printf(ARGS_MISSING_POSITIONAL_TEXT, "NUM");
//                 break;
//             case ARGS_EXPECTS_STRING:
//                 printf(ARGS_MISSING_POSITIONAL_TEXT, "STR");
//                 break;
//             case ARGS_EXPECTS_FILE:
//                 printf(ARGS_MISSING_POSITIONAL_TEXT, "FILE");
//                 break;
//             default:
//                 printf(ARGS_MISSING_POSITIONAL_TEXT, "ARG");
//         }
//         if (positional_type == ARGS_POSITIONAL_MULTI) printf("...");
//         putchar('\n');
//         #ifndef ARGS_HELP_FLAG_DISABLED
//         printf(ARGS_USAGE_ERR_HELP_TEXT, ARGS_BINARY_NAME, ARGS_HELP_FLAG_NAME_LONG);
//         putchar('\n');
//         #endif // ARGS_HELP_FLAG_DISABLED
//         return EX_USAGE;
//     }
//
//     return ARGS_RETURN_CONTINUE;
// }
//
// #endif // ARGS_IMPLEMENTATION
//
