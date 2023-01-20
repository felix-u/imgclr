/* args.h - easy and robust command line argument parsing - felix-u 2022
*
*   This is an stb-style single-header-file C library.
*
*   To use this library, do this in *one* C file:
*       #define ARGS_IMPLEMENTATION
*       #include "args.h"
*
*
*   COMPILE-TIME OPTIONS
*
*   #define ARGS_BINARY_NAME "name"
*       If this is not defined, argv[0] will be used instead.
*
*   #define ARGS_BINARY_VERSION "1.0"
*       *Required* to use ARGS_VERSION_FLAG.
*
*   #define ARGS_HELP_FLAG_DISABLED
*       Disables the provided ARGS_HELP_FLAG.
*
*   Other options not expected to change but able to be modified are in the source code.
*
*
*   DOCUMENTATION
*
*   Basic usage (see EXAMPLE for a full example):
*       args_Flag *flags[] = { ... };
*       size_t positional_num = 0;
*       const size_t positional_cap = LARGE_NUMBER;
*       char *positional_args[positional_cap];
*       int args_return = args_process(argc, argv, "short description of program", flags_count, flags, &positional_num,
*                                      positional_args, positional_expects, positional_type, positional_cap);
*       if (args_return != ARGS_RETURN_CONTINUE) return args_return;
*
*   See near top of source code for enum types.
*
*   args_process() parameters:
*       int argc            -- from main()
*       char **argv         -- from main()
*       const char *usage_description                      -- short description of program purpose, shown at the top of
*                                                             help text
*       const size_t flags_count                           -- number of flags
*       args_flag *flags[]                                 -- flags
*       size_t *positional_num                             -- number of positional arguments passed directly to the
*                                                             binary. Unknown before processing, and therefore 0
*       char **positional_args                             -- empty string array of size positional_cap. Will contain
*                                                             positional arguments passed directly to the binary
*       const ARGS_FLAG_EXPECTS positional_expects         -- what kind of positional arguments the binary expects.
*                                                             Used in help text
*       const ARGS_BINARY_POSITIONAL_TYPE positional_type  -- how many positional arguments the binary expects
*       const size_t positional_cap                        -- max no. of positional arguments; size of positional_args
*
*   struct args_Flag members:
*       const char name_short               -- flag short form, e.g. '-h'. Can be false
*       const char *name_long               -- flag long form, e.g. '--help'. Must *not* be NULL
*       const char *help_text               -- flag help text, e.g. 'display this help'. May be NULL, but shouldn't
*       const bool required                 -- whether flag is required
*       bool is_present                     -- whether flag has been passed. Should initially be false
*       char **opts                         -- arguments passed to flag. *Must* initially be NULL
*       size_t opts_num                     -- number of arguments passed to flag. *Must* initially be 0
*       const ARGS_FLAG_TYPE type           -- whether the flag is boolean, expects one argument, or expects multiple
*       const ARGS_FLAG_EXPECTS expects     -- what kind of arguments the flag expects. Used in help text
*
*
*   EXAMPLE
*
*       #include <stdio.h>
*       #include <stdlib.h>
*
*       #define ARGS_IMPLEMENTATION
*       #define ARGS_BINARY_NAME "name"
*       #define ARGS_BINARY_VERSION "version"
*       #include "args.h"
*
*       int main(int argc, char **argv) {
*
*           args_Flag boolean_flag = {
*               'b', "boolean",
*               "an example boolean flag",
*               ARGS_OPTIONAL,
*               false, NULL, 0,
*               ARGS_BOOLEAN, ARGS_EXPECTS_NONE
*           };
*           args_Flag single_flag = {
*               's', "single",
*               "an example of a flag which takes one argument",
*               ARGS_REQUIRED,
*               false, NULL, 0,
*               ARGS_SINGLE_OPT, ARGS_EXPECTS_STRING
*           };
*           args_Flag multi_flag = {
*               'm', "multi",
*               "an example of a flag which takes multiple arguments",
*               ARGS_REQUIRED,
*               false, NULL, 0,
*               ARGS_MULTI_OPT, ARGS_EXPECTS_FILE
*           };
*           args_Flag *flags[] = {
*               &boolean_flag,
*               &single_flag,
*               &multi_flag,
*               ARGS_HELP_FLAG,
*               ARGS_VERSION_FLAG,
*           };
*
*           const size_t flags_count = sizeof(flags) / sizeof(flags[0]);
*           size_t positional_num = 0;
*           const size_t positional_cap = 256;
*           char *positional_args[positional_cap];
*
*           int args_return = args_process(argc, argv, "test of args.h", flags_count, flags, &positional_num,
*                                          positional_args, ARGS_EXPECTS_FILE, ARGS_POSITIONAL_MULTI, positional_cap);
*           if (args_return != ARGS_RETURN_CONTINUE) return args_return;
*
*           if (single_flag.is_present) printf("You passed flag 'single'!\n");
*
*           return 0;
*       }
*
*/


#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>


#ifndef ARGS_TYPE
#define ARGS_TYPE

typedef enum ARGS_FLAG_TYPE {
    ARGS_NONE,
    ARGS_BOOLEAN,
    ARGS_SINGLE_OPT,
    ARGS_MULTI_OPT,
} ARGS_FLAG_TYPE;

typedef enum ARGS_BINARY_POSITIONAL_TYPE {
    ARGS_POSITIONAL_SINGLE,
    ARGS_POSITIONAL_MULTI,
} ARGS_BINARY_POSITIONAL_TYPE;

typedef enum ARGS_FLAG_EXPECTS {
    ARGS_EXPECTS_NONE,
    ARGS_EXPECTS_NUM,
    ARGS_EXPECTS_STRING,
    ARGS_EXPECTS_FILE,
} ARGS_FLAG_EXPECTS;

typedef enum ARGS_FLAG_REQUIRED {
    ARGS_REQUIRED = true,
    ARGS_OPTIONAL = false
} ARGS_FLAG_REQUIRED;

typedef struct args_Flag {
    const char name_short;
    const char *name_long;
    const char *help_text;
    const bool required;
    bool is_present;
    char **opts;
    size_t opts_num;
    const ARGS_FLAG_TYPE type;
    const ARGS_FLAG_EXPECTS expects;
} args_Flag;

#endif // ARGS_TYPE


args_Flag *args_byNameShort(char name_short, const size_t flags_count, args_Flag *flags[]);

args_Flag *args_byNameLong(char *name_long, const size_t flags_count, args_Flag *flags[]);

void args_helpHint(void);

bool args_optionalFlagsPresent(const size_t flags_count, args_Flag *flags[]);

int args_process
(int argc, char **argv, const char *usage_description, const size_t flags_count, args_Flag *flags[],
size_t *positional_num, char **positional_args, const ARGS_FLAG_EXPECTS positional_expects,
const ARGS_BINARY_POSITIONAL_TYPE positional_type, const size_t positional_cap);


#ifdef ARGS_IMPLEMENTATION

#define EX_USAGE 64

#ifndef ARGS_RETURN_CONTINUE
#define ARGS_RETURN_CONTINUE -1
#endif // ARGS_RETURN_CONTINUE

#ifndef ARGS_BINARY_NAME
#define ARGS_BINARY_NAME argv[0]
#endif // ARGS_BINARY_NAME

#ifndef ARGS_MISSING_FLAG_TEXT
#define ARGS_MISSING_FLAG_TEXT "option '--%s' is required"
#endif // ARGS_MISSING_FLAG_TEXT

#ifndef ARGS_MISSING_ARG_TEXT
#define ARGS_MISSING_ARG_TEXT "option '--%s' requires an argument"
#endif // ARGS_MISSING_ARG_TEXT

#ifndef ARGS_INVALID_FLAG_TEXT
#define ARGS_INVALID_FLAG_TEXT "invalid option"
#endif // ARGS_INVALID_FLAG_TEXT

#ifndef ARGS_MISSING_POSITIONAL_TEXT
#define ARGS_MISSING_POSITIONAL_TEXT "expected %s"
#endif // ARGS_MISSING_POSITIONAL_TEXT

#ifndef ARGS_HELP_FLAG_DISABLED
    #ifndef ARGS_USAGE_ERR_HELP_TEXT
    #define ARGS_USAGE_ERR_HELP_TEXT "Try '%s --%s' for more information."
    #endif // ARGS_USAGE_ERR_HELP_TEXT

    #ifndef ARGS_HELP_FLAG_NAME_SHORT
    #define ARGS_HELP_FLAG_NAME_SHORT 'h'
    #endif // ARGS_HELP_FLAG_NAME_SHORT

    #ifndef ARGS_HELP_FLAG_NAME_LONG
    #define ARGS_HELP_FLAG_NAME_LONG "help"
    #endif // ARGS_HELP_FLAG_NAME_LONG

    #ifndef ARGS_HELP_FLAG_HELP_TEXT
    #define ARGS_HELP_FLAG_HELP_TEXT "display this help and exit"
    #endif // ARGS_HELP_FLAG_HELP_TEXT

    #ifndef ARGS_HELP_FLAG
    #define ARGS_HELP_FLAG (struct args_Flag){  \
        ARGS_HELP_FLAG_NAME_SHORT,              \
        ARGS_HELP_FLAG_NAME_LONG,               \
        ARGS_HELP_FLAG_HELP_TEXT,               \
        ARGS_OPTIONAL,                          \
        false, NULL, 0,                         \
        ARGS_BOOLEAN, ARGS_EXPECTS_NONE         \
    }
    #endif // ARGS_HELP_FLAG
#endif // ARGS_HELP_FLAG_DISABLED

#ifndef ARGS_BINARY_VERSION
#define ARGS_VERSION_FLAG_DISABLED
#endif // ARGS_BINARY_VERSION

#ifndef ARGS_VERSION_FLAG_DISABLED
    #ifndef ARGS_VERSION_FLAG_NAME_SHORT
    #define ARGS_VERSION_FLAG_NAME_SHORT false
    #endif // ARGS_VERSION_FLAG_NAME_SHORT

    #ifndef ARGS_VERSION_FLAG_NAME_LONG
    #define ARGS_VERSION_FLAG_NAME_LONG "version"
    #endif // ARGS_VERSION_FLAG_NAME_LONG

    #ifndef ARGS_VERSION_FLAG_HELP_TEXT
    #define ARGS_VERSION_FLAG_HELP_TEXT "output version information and exit"
    #endif // ARGS_VERSION_FLAG_HELP_TEXT

    #ifndef ARGS_VERSION_FLAG
    #define ARGS_VERSION_FLAG (struct args_Flag){  \
        ARGS_VERSION_FLAG_NAME_SHORT,              \
        ARGS_VERSION_FLAG_NAME_LONG,               \
        ARGS_VERSION_FLAG_HELP_TEXT,               \
        ARGS_OPTIONAL,                             \
        false, NULL, 0,                            \
        ARGS_BOOLEAN, ARGS_EXPECTS_NONE            \
    }
    #endif // ARGS_VERSION_FLAG
#endif // ARGS_VERSION_FLAG_DISABLED


args_Flag *args_byNameShort(char name_short, const size_t flags_count, args_Flag *flags[]) {
    for (size_t i = 0; i < flags_count; i++) {
        if (name_short == flags[i]->name_short) return flags[i];
    }
    return NULL;
}


args_Flag *args_byNameLong(char *name_long, const size_t flags_count, args_Flag *flags[]) {
    for (size_t i = 0; i < flags_count; i++) {
        if (!strncasecmp(name_long, flags[i]->name_long, strlen(name_long))) return flags[i];
    }
    return NULL;
}


void args_helpHint(void) {
    #ifndef ARGS_HELP_FLAG_DISABLED
    printf(ARGS_USAGE_ERR_HELP_TEXT, ARGS_BINARY_NAME, ARGS_HELP_FLAG_NAME_LONG);
    putchar('\n');
    #endif // ARGS_HELP_FLAG_DISABLED
}


bool args_optionalFlagsPresent(const size_t flags_count, args_Flag *flags[]) {
    for (size_t i = 0; i < flags_count; i++) {
        if (!flags[i]->required && flags[i]->is_present) return true;
    }
    return false;
}


int args_process
(int argc, char **argv, const char *usage_description, const size_t flags_count, args_Flag *flags[],
size_t *positional_num, char **positional_args, const ARGS_FLAG_EXPECTS positional_expects,
const ARGS_BINARY_POSITIONAL_TYPE positional_type, const size_t positional_cap)
{
    #ifdef ARGS_HELP_FLAG_DISABLED
    (void) usage_description;
    #endif // ARGS_HELP_FLAG_DISABLED

    // All flags MUST have a long format, if not a short one.
    // While we're looping over flags, if none are required, let's not mention mandatory options in help text later.
    bool any_mandatory = false;
    for (size_t i = 0; i < flags_count; i++) {
        assert(flags[i]->name_long != NULL && "One flag has a NULL name_long");
        if (flags[i]->required == ARGS_REQUIRED) any_mandatory = true;
    }

    #ifndef ARGS_HELP_FLAG_DISABLED
    args_Flag *help_flag = args_byNameShort(ARGS_HELP_FLAG_NAME_SHORT, flags_count, flags);
    bool help_implied = false;
    #endif // ARGS_HELP_FLAG_DISABLED

    // Immediately show help if binary was expecting positional arguments but got none.
    #ifndef ARGS_HELP_FLAG_DISABLED
    int help_return = EX_USAGE;
    if ((positional_type == ARGS_POSITIONAL_SINGLE || positional_type == ARGS_POSITIONAL_MULTI) && argc == 1) {
        if (help_flag != NULL) help_flag->is_present = true;
        help_implied = true;
    }
    #endif // ARGS_HELP_FLAG_DISABLED

    int skip = 0;
    bool no_more_flags = false; // All arguments are positional after "--", as is convention.
    bool is_positional[argc];
    for (int i = 0; i < argc; i++) is_positional[i] = false;
    *positional_num = 0;

    for (int i = 1; i + skip < argc; i++) {
        i += skip;
        skip = 0;

        char *arg = argv[i];
        size_t arg_len = strlen(arg);

        if ((arg[0] != '-' || no_more_flags) && (size_t)i <= positional_cap) {
            (*positional_num)++;
            is_positional[i] = true;
            continue;
        }

        // @Note { There is no convention for dealing with arg = '-'. I skip it. }

        // Arg starts with "--"
        if (arg_len > 1 && arg[1] == '-') {

            // Double dash ("--") means only positionals beyond this point!
            if (arg_len == 2) {
                no_more_flags = true;
                continue;
            }

            // Arg is of form "--arg"
            bool found_match = false;
            for (size_t j = 0; j < flags_count; j++) {
                if (!strncasecmp(flags[j]->name_long, (arg + 2), (arg_len))) {
                    flags[j]->is_present = true;
                    if (flags[j]->type == ARGS_SINGLE_OPT && (i + 1) < argc &&
                       (strlen(argv[i + 1]) >= 1 && argv[i + 1][0] != '-'))
                    {
                        flags[j]->opts = argv + i + 1;
                        flags[j]->opts_num = 1;
                        skip = 1;
                    }
                    else if (flags[j]->type == ARGS_MULTI_OPT) {
                        size_t opts_num = 0;
                        for (int k = i + 1; k < argc; k++) {
                            if (strlen(argv[k]) > 1 && argv[k][0] == '-') break;
                            opts_num++;
                        }
                        flags[j]->opts = argv + i + 1;
                        flags[j]->opts_num = opts_num;
                        skip = opts_num;
                    }
                    found_match = true;
                    break;
                }
            }
            // Flag invalid
            if (!found_match) {
                printf("%s: %s '%s'\n", ARGS_BINARY_NAME, ARGS_INVALID_FLAG_TEXT, argv[i]);
                #ifndef ARGS_HELP_FLAG_DISABLED
                printf(ARGS_USAGE_ERR_HELP_TEXT, ARGS_BINARY_NAME, ARGS_HELP_FLAG_NAME_LONG);
                putchar('\n');
                #endif // ARGS_HELP_FLAG_DISABLED
                return EX_USAGE;
            }

            continue;
        }

        // Arg is of form "-arg"

        // Go up to last character
        for (size_t j = 1; j < arg_len - 1; j++) {
            bool found_match = false;
            char invalid_short_arg = 0;
            for (size_t k = 0; k < flags_count; k++) {
                if (arg[j] == flags[k]->name_short) {
                    flags[k]->is_present = true;
                    found_match= true;
                    break;
                }
            }
            // Flag invalid
            if (!found_match) {
                printf("%s: %s '%c' in '%s'\n", ARGS_BINARY_NAME, ARGS_INVALID_FLAG_TEXT, arg[j], arg);
                #ifndef ARGS_HELP_FLAG_DISABLED
                printf(ARGS_USAGE_ERR_HELP_TEXT, ARGS_BINARY_NAME, ARGS_HELP_FLAG_NAME_LONG);
                putchar('\n');
                #endif // ARGS_HELP_FLAG_DISABLED
                return EX_USAGE;
            }
        }
        // Last character could have options supplied to it ("-arg opt opt" == "-a -r -g opt opt").
        bool found_match = false;
        for (size_t k = 0; k < flags_count; k++) {
            if (arg[arg_len - 1] == flags[k]->name_short) {
                flags[k]->is_present = true;
                found_match = true;
                if (flags[k]->type == ARGS_SINGLE_OPT && (i + 1) < argc &&
                   (strlen(argv[i + 1]) >= 1 && argv[i + 1][0] != '-'))
                {
                    flags[k]->opts = argv + i + 1;
                    flags[k]->opts_num = 1;
                    skip = 1;
                }
                else if (flags[k]->type == ARGS_MULTI_OPT) {
                    size_t opts_num = 0;
                    for (int l = i + 1; l < argc; l++) {
                        if (strlen(argv[l]) > 1 && argv[l][0] == '-') break;
                        opts_num++;
                    }
                    flags[k]->opts = argv + i + 1;
                    flags[k]->opts_num = opts_num;
                    skip = opts_num;
                }
            }
        }
        // Flag invalid
        if (!found_match) {
            if (arg_len > 2) {
                printf("%s: %s '%c' in '%s'\n", ARGS_BINARY_NAME, ARGS_INVALID_FLAG_TEXT, arg[arg_len - 1], arg);
            }
            else printf("%s: %s '%s'\n", ARGS_BINARY_NAME, ARGS_INVALID_FLAG_TEXT, arg);

            #ifndef ARGS_HELP_FLAG_DISABLED
            printf(ARGS_USAGE_ERR_HELP_TEXT, ARGS_BINARY_NAME, ARGS_HELP_FLAG_NAME_LONG);
            putchar('\n');
            #endif // ARGS_HELP_FLAG_DISABLED
            return EX_USAGE;
        }
    }

    // Fill array of positional args
    size_t positional_counter = 0;
    for (int i = 1; i < argc; i++) {
        if (is_positional[i] == true && positional_counter < positional_cap) {
            positional_args[positional_counter] = argv[i];
            positional_counter++;
        }
    }

    // Help text
    #ifndef ARGS_HELP_FLAG_DISABLED
    if (help_flag != NULL && help_flag->is_present == true) {
        printf(ARGS_BINARY_NAME);
        if (usage_description != NULL) printf(" - %s", usage_description);
        #ifndef ARGS_VERSION_FLAG_DISABLED
        printf(" (version %s)", ARGS_BINARY_VERSION);
        #endif // ARGS_VERSION_FLAG_DISABLED

        printf("\n\nUSAGE:\n");
        printf("%s <OPTION>", ARGS_BINARY_NAME);
        if (flags_count > 1) printf("...");
        if (positional_expects != ARGS_EXPECTS_NONE) {
            switch (positional_expects) {
                case ARGS_EXPECTS_NUM:
                    printf(" <NUM>");
                    break;
                case ARGS_EXPECTS_STRING:
                    printf(" <STR>");
                    break;
                case ARGS_EXPECTS_FILE:
                    printf(" <FILE>");
                    break;
                default:
                    printf(" <ARG>");
            }
            if (positional_type == ARGS_POSITIONAL_MULTI) printf("...");
        }
        putchar('\n');
        if (any_mandatory) printf("Options marked with '*' are mandatory.\n");

        printf("\nOPTIONS:\n");
        for (size_t i = 0; i < flags_count; i++) {
            if (flags[i]->required == true) printf("* ");
            else printf("  ");

            if (flags[i]->name_short != false) printf("-%c, ", flags[i]->name_short);
            else printf("    ");

            if (flags[i]->name_long != NULL) printf("--%s", flags[i]->name_long);

            if (flags[i]->expects != ARGS_EXPECTS_NONE) {
                switch (flags[i]->expects) {
                    case ARGS_EXPECTS_NUM:
                        printf(" <NUM>");
                        break;
                    case ARGS_EXPECTS_STRING:
                        printf(" <STR>");
                        break;
                    case ARGS_EXPECTS_FILE:
                        printf(" <FILE>");
                        break;
                    default:
                        printf(" <ARG>");
                }
                if (flags[i]->type == ARGS_MULTI_OPT) printf("...");
            }

            putchar('\n');

            if (flags[i]->help_text != NULL) {
                putchar('\t');
                size_t help_len = strlen(flags[i]->help_text);
                for (size_t j = 0; j < help_len; j++) {
                    putchar(flags[i]->help_text[j]);
                    if (flags[i]->help_text[j] == '\n') putchar('\t');
                }
                putchar('\n');
            }
        }

        if (help_implied) return EX_USAGE;
        return EXIT_SUCCESS;
    }
    #endif // ARGS_HELP_FLAG_DISABLED

    // Version number
    #ifndef ARGS_VERSION_FLAG_DISABLED
    args_Flag *version_flag = args_byNameLong(ARGS_VERSION_FLAG_NAME_LONG, flags_count, flags);
    if (version_flag != NULL && version_flag->is_present) {
        printf("%s %s\n", ARGS_BINARY_NAME, ARGS_BINARY_VERSION);
        return EXIT_SUCCESS;
    }
    #endif // ARGS_VERSION_FLAG_DISABLED

    // Check mandatory flags
    for (size_t i = 0; i < flags_count; i++) {
        if (flags[i]->required && !flags[i]->is_present) {
            printf("%s: ", ARGS_BINARY_NAME);
            printf(ARGS_MISSING_FLAG_TEXT, flags[i]->name_long);
            putchar('\n');
            #ifndef ARGS_HELP_FLAG_DISABLED
            printf(ARGS_USAGE_ERR_HELP_TEXT, ARGS_BINARY_NAME, ARGS_HELP_FLAG_NAME_LONG);
            putchar('\n');
            #endif // ARGS_HELP_FLAG_DISABLED
            return EX_USAGE;
        }
        else if (flags[i]->is_present && (flags[i]->type == ARGS_SINGLE_OPT || flags[i]->type == ARGS_MULTI_OPT)
                 && flags[i]->opts_num < 1)
        {
            printf("%s: ", ARGS_BINARY_NAME);
            printf(ARGS_MISSING_ARG_TEXT, flags[i]->name_long);
            putchar('\n');
            #ifndef ARGS_HELP_FLAG_DISABLED
            printf(ARGS_USAGE_ERR_HELP_TEXT, ARGS_BINARY_NAME, ARGS_HELP_FLAG_NAME_LONG);
            putchar('\n');
            #endif // ARGS_HELP_FLAG_DISABLED
            return EX_USAGE;
        }
    }

    // Exit if positional arguments were expected, but not received.
    if ((positional_type == ARGS_POSITIONAL_SINGLE || positional_type == ARGS_POSITIONAL_MULTI)
        && *positional_num == 0)
    {
        printf("%s: ", ARGS_BINARY_NAME);
        switch (positional_expects) {
            case ARGS_EXPECTS_NUM:
                printf(ARGS_MISSING_POSITIONAL_TEXT, "NUM");
                break;
            case ARGS_EXPECTS_STRING:
                printf(ARGS_MISSING_POSITIONAL_TEXT, "STR");
                break;
            case ARGS_EXPECTS_FILE:
                printf(ARGS_MISSING_POSITIONAL_TEXT, "FILE");
                break;
            default:
                printf(ARGS_MISSING_POSITIONAL_TEXT, "ARG");
        }
        if (positional_type == ARGS_POSITIONAL_MULTI) printf("...");
        putchar('\n');
        #ifndef ARGS_HELP_FLAG_DISABLED
        printf(ARGS_USAGE_ERR_HELP_TEXT, ARGS_BINARY_NAME, ARGS_HELP_FLAG_NAME_LONG);
        putchar('\n');
        #endif // ARGS_HELP_FLAG_DISABLED
        return EX_USAGE;
    }

    return ARGS_RETURN_CONTINUE;
}

#endif // ARGS_IMPLEMENTATION
