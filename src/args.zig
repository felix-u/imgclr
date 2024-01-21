// Last tested with zig 0.12.0-dev.2139+e025ad7b4 on 2024-01-11
//
// args.zig - public domain command-line argument parser - felix-u
//
// No warranty implied; use at your own risk.
// See end of file for licence information.
//
//
// Documentation contains:
// * Summary
// * Usage
// * Guarantees
// * Notes
// * Examples
//
// For a quick start, view the examples, then refer to Usage as needed.
//
// Throughout this documentation, `args_parsed` refers to what is returned by
// `args_parseAlloc(...)`.
//
//
// Summary:
//
// args.zig parses command-line arguments, with support for conventional syntax
// and single-level subcommands.
//
// The following are equivalent uses of a hypothetical programmer-defined
// interface, as parsed by args.zig:
// * `exe --verbose --feature 1 2 3`
// * `exe -vf1 2 3`
// * `exe --verbose -f 1 2 3`
// * `exe -v --feature=1 --feature=2 3`
//
// args.zig creates `-h`/`--help` flags at the general and (if applicable)
// subcommand levels.
// If `ParseParams.ver` is set, args.zig creates a `--version` flag at the
// general level.
//
//
// Usage:
//
// args.parseAlloc(
//     allocator: std.mem.Allocator
//     writer: std.fs.File.Writer - usually stdout
//     err_writer: std.fs.File.Writer - usually stderr
//     argv: [][]const u8
//     comptime p: ParseParams - see below
// )
//
// struct ParseParams {
//     desc: []const u8 - program description (can leave empty)
//     ver: []const u8 - program version (can leave empty)
//     usage: []const u8 - program usage (can leave empty)
//     cmds: []const Cmd = &.{} - at least one (1) (sub)command - see below
//     err_msg: bool - whether to print error messages (default: true)
// }
//
// struct Cmd {
//     kind: Kind - .boolean, .single_pos, or .multi_pos (default: .boolean)
//     desc: []const u8 - subcommand description (can leave empty)
//     usage: []const u8 - subcommand usage (can leave empty)
//     name: []const u8 - subcommand name (must set)
//     flags: []const Flag - any number of flags (can leave empty) - see below
// }
//
// struct Flag {
//     short: u8 - flag short form, as in `-f` (can leave none)
//     long: []const u8 - flag long form, as in `--flag` (must set)
//     kind: Kind - .boolean, .single_pos, or .multi_pos (default: .boolean)
//     required: bool - whether to enforce usage of flag (default: false)
//     desc: []const u8 - flag description (can leave empty)
//     usage: []const u8 - flag usage (can leave empty)
// }
//
// If your program has subcommands, define multiple commands. Otherwise, define
// one (1).
//
// Command results are accessed through `args_parsed.command_name`, which is
// nullable if the number of commands provided is greater than one (1). This
// field has the following subfields:
// * `.invoked: bool` - always true for single commands;
// * `.pos` - `bool` if `cmd.kind = .boolean`, `[]const u8` if `cmd.kind =
//   .single_pos`, and `std.Arraylist([]const u8)` if `cmd.kind = .multi_pos`;
// * Another field for each flag - see next paragraph.
//
// Flag results are accessed through `args_parsed.command_name.flag_long_form`.
// The type of this field is:
// * `bool` if `flag.kind = .boolean`;
// * `[]const u8` if `flag.kind = .single_pos`, and nullable if `flag.required
//   = false`;
// * `std.ArrayList([]const u8)` if `flag.kind = .multi_pos`.
// Therefore, to test for the presence of a non-required flag, either check:
// * if its field is `true`;
// * if its field is not null;
// * if field.items.len > 0;
// based on the above types.
//
//
// Guarantees:
//
// * `args.parseAlloc()` will return an error if incorrect usage was detected,
//   null if `--help` or `--version` were called, and a comptime-generated
//   result type otherwise. See Examples.
//
// * If there are multiple subcommands, exactly one (1) in the result type will
//   have `.invoked = true`.
//
// * args.parseAlloc() ensures the following:
//     - Flags and commands with `.kind = .single_pos` have received exactly
//       one (1) positional argument from the user.
//     - Flags with `.required = true` have been provided by the user.
//     - No invalid subcommands or flags have been provided by the user.
//
//   If any of the above conditions is not met, args.parseAlloc() will:
//     1) print a helpful error messages, unless `ParseParams.err_msg = false`;
//     2) return an error.
//
//
// Notes:
//
// * Every flag MUST have a `.long` form: its results are accessed using
//   `args_parsed.cmd_name.long_flag_form`. Its `.short` form may be left
//   unset.
//
// * Flags and commands with `.kind = .multi_pos` can receive any number of
//   positional arguments, including zero (0).
//
// * If there is only one (1) `args.Cmd`, it is not considered a subcommand.
//   Leave its `.desc` and `.usage` empty and use `ParseParams.desc` and
//   `ParseParams.usage` instead. The `.name` of a single `args.Cmd` is not
//   used at runtime, but is used by the programmer to access the fields of the
//   command's result type, i.e. `args_parsed.cmd_name.pos.items`. A single
//   `args.Cmd` is guaranteed to have `.invoked = true` - this does not need to
//   be checked.
//
// * The default error messages can be modified by editing the `errMsg`
//   function.
//
// * If `ParseParams.ver` is not set to a non-empty string, no `--version` flag
//   will be generated.
//
// * `ParseParams.usage` and `Cmd.usage` are for the help text - you can set
//   them to whatever you like, or leave them empty. When printed, they are
//   preceded by the binary or command names.
//
//
// Example: `ls`
// ------
// const std = @import("std");
// const args = @import("args.zig");
//
// pub fn main() !void {
//     const allocator = ...;
//     const stdout = std.io.getStdOut().writer();
//     const stderr = std.io.getStdErr().writer();
//
//     const argv = try std.process.argsAlloc(allocator);
//     defer std.process.argsFree(allocator, argv);
//
//     const args_parsed = args.parseAlloc(allocator, stdout, stderr, argv, .{
//         .desc = "list contents of directory",
//         .usage = "[ -lrF ] [paths]" ,
//         .cmds = &.{args.Cmd{
//             .name = "ls",
//             .kind = .multi_pos,
//             .flags = &.{
//                 args.Flag{
//                     .short = 'l',
//                     .long = "long",
//                     .desc = "List in long format",
//                 },
//                 args.Flag{
//                     .short = 'r',
//                     .long = "reverse",
//                     .desc = "Reverse the sort order",
//                 },
//                 args.Flag{
//                     .short = 'F',
//                     .long = "format",
//                     .desc = "Add '/' after directory names",
//                 },
//             },
//         }},
//     }) catch {
//         std.os.exit(1);
//     } orelse return;
//     defer allocator.destroy(args_parsed);
//
//     for (args_parsed.ls.pos) |path| {
//         ...
//     }
// }
// ------
//
// Output of `ls --help`:
// -----
// ls - list contents of directory
//
// Usage:
//   ls [ -lrF ] [paths]
//
// Options:
//   -l, --long
//       List in long format
//   -r, --reverse
//       Reverse the sort order
//   -F, --format
//       Add '/' after directory names and '*' after executables
//   -h, --help
//       Print this help and exit
//
// -----
//
// Example: `git`
// -----
// ...
//
// pub fn main() !void {
//     ...
//
//     const args_parsed = args.parseAlloc(allocator, stdout, stderr, argv, .{
//         .desc = "distributed revision control system",
//         .usage = "<command> [args]",
//         .cmds = &.{
//             args.Cmd{
//                 .desc = "Clone a repository",
//                 .usage = "[flags] <repository>",
//                 .name = "clone",
//                 .kind = .single_pos,
//                 .flags = &.{
//                     args.Flag{
//                         .short = 'q',
//                         .long = "quiet",
//                         .desc = "Operate quietly",
//                     },
//                     args.Flag{
//                         .long = "depth",
//                         .kind = .single_pos,
//                         .desc = "Create a shallow clone",
//                         .usage = "<depth>",
//                     },
//                 },
//             },
//             args.Cmd{
//                 .desc = "Create an empty Git repository",
//                 .usage = "[flags] [directory]",
//                 .name = "init",
//                 .kind = .multi_pos,
//                 .flags = &.{
//                     args.Flag{
//                         .long = "bare",
//                         .desc = "Create a bare repository",
//                     },
//                     args.Flag{
//                         .long = "separate-git-dir",
//                         .kind = .single_pos,
//                         .desc = "Create a Git symbolic link",
//                         .usage = "<git-dir>",
//                     },
//                 },
//             },
//         },
//     }) catch {
//         std.os.exit(1);
//     } orelse return;
//     defer allocator.destroy(args_parsed);
//
//     if (args_parsed.clone.invoked) {
//         ...
//     }
// }
// -----
//
// Output of `git --help`:
// -----
// git - distributed revision control system
//
// Usage:
//   git <command> [args]
//
// Commands:
//   clone [flags] <repository>
//       Clone a repository
//   init [flags] [directory]
//       Create an empty Git repository
//
// General options:
//   -h, --help
//       Print this help and exit
//
// -----
//
// Output of `git init --help`:
// -----
// init - Create an empty Git repository
//
// Usage:
//   git init [flags] [directory]
//
// Options:
//       --bare
//       Create a bare repository
//       --separate-git-dir <git-dir>
//       Instead of creating ./.git/, create a Git symbolic link
//   -h, --help
//       Print this help and exit
//
// -----

const std = @import("std");
const Writer = std.fs.File.Writer;

pub const Err = error{
    InvalidCmd,
    InvalidFlag,
    InvalidUsage,
    MissingArg,
    MissingCmd,
    MissingFlag,
    UnexpectedArg,
};

pub fn errMsg(
    comptime print: bool,
    comptime e: Err,
    err_writer: Writer,
    info: []const u8,
) anyerror {
    if (!print) return e;

    _ = try err_writer.write("error: ");

    switch (e) {
        inline Err.InvalidCmd => {
            try err_writer.print("no such command '{s}'", .{info});
        },
        inline Err.InvalidFlag => {
            try err_writer.print("no such flag '{s}'", .{info});
        },
        inline Err.InvalidUsage => _ = try err_writer.write("invalid usage"),
        inline Err.MissingArg => try err_writer.print(
            "expected positional argument to '{s}'",
            .{info},
        ),
        inline Err.MissingCmd => {
            _ = try err_writer.write("expected command");
        },
        inline Err.MissingFlag => _ = try err_writer.write("expected flag"),
        inline Err.UnexpectedArg => try err_writer.print(
            "unexpected positional argument '{s}'",
            .{info},
        ),
    }

    try err_writer.writeByte('\n');

    return e;
}

pub const Kind = enum(u8) { boolean, single_pos, multi_pos };

pub const Flag = struct {
    short: u8 = 0,
    long: []const u8 = "",

    kind: Kind = .boolean,
    required: bool = false,
    desc: []const u8 = "",
    usage: []const u8 = "",

    fn resultType(comptime self: *const @This()) type {
        switch (self.kind) {
            inline .boolean => return bool,
            inline .single_pos => {
                return if (self.required) []const u8 else ?[]const u8;
            },
            inline .multi_pos => return std.ArrayList([]const u8),
        }
    }
};

const StructField = std.builtin.Type.StructField;

pub const Cmd = struct {
    kind: Kind = .boolean,
    desc: []const u8 = "",
    usage: []const u8 = "",
    name: []const u8,

    flags: []const Flag = &.{},

    fn resultType(comptime self: *const @This()) type {
        comptime var fields: [self.flags.len + 2]StructField = undefined;
        fields[0] = .{
            .name = "invoked",
            .type = bool,
            .default_value = &false,
            .is_comptime = false,
            .alignment = 0,
        };

        fields[1] = .{
            .name = "pos",
            .type = switch (self.kind) {
                inline .boolean => bool,
                inline .single_pos => []const u8,
                inline .multi_pos => std.ArrayList([]const u8),
            },
            .default_value = null,
            .is_comptime = false,
            .alignment = 0,
        };

        inline for (fields[2..], self.flags) |*field, flag| {
            field.* = .{
                .name = @ptrCast(flag.long),
                .type = flag.resultType(),
                .default_value = null,
                .is_comptime = false,
                .alignment = 0,
            };
        }

        return @Type(.{ .Struct = .{
            .layout = .Auto,
            .fields = &fields,
            .decls = &.{},
            .is_tuple = false,
        } });
    }

    fn listResultType(comptime cmds: []const @This()) type {
        if (cmds.len == 0) return void;

        comptime var fields: [cmds.len]StructField = undefined;
        inline for (&fields, cmds) |*field, cmd| {
            field.* = .{
                .name = @ptrCast(cmd.name),
                .type = cmd.resultType(),
                .default_value = null,
                .is_comptime = false,
                .alignment = 0,
            };
        }

        return @Type(.{ .Struct = .{
            .layout = .Auto,
            .fields = &fields,
            .decls = &.{},
            .is_tuple = false,
        } });
    }
};

pub const help_flag = Flag{
    .short = 'h',
    .long = "help",
    .desc = "Print this help and exit",
};

pub const ver_flag = Flag{
    .long = "version",
    .desc = "Print version information and exit",
};

pub const ParseParams = struct {
    desc: []const u8 = "",
    ver: []const u8 = "",
    usage: []const u8 = "",
    cmds: []const Cmd = &.{},
    err_msg: bool = true,
};

pub fn parseAlloc(
    allocator: std.mem.Allocator,
    writer: Writer,
    err_writer: Writer,
    argv: [][]const u8,
    comptime p: ParseParams,
) !?*Cmd.listResultType(p.cmds) {
    if (argv.len == 1) {
        try printHelp(err_writer, argv[0], p, null);
        return errMsg(p.err_msg, Err.InvalidUsage, err_writer, &.{});
    }

    const Result = Cmd.listResultType(p.cmds);
    var result = try allocator.create(Result);
    errdefer allocator.destroy(result);

    inline for (@typeInfo(Result).Struct.fields, 0..) |_, i| {
        const cmd = p.cmds[i];

        if (cmd.kind == .multi_pos) {
            const array_list = std.ArrayList([]const u8).init(allocator);
            const flag_list = &@field(result, cmd.name).pos;
            flag_list.* = array_list;
            try flag_list.ensureTotalCapacity(argv.len);
        }

        inline for (cmd.flags) |flag| {
            if (flag.kind != .multi_pos) continue;
            const array_list = std.ArrayList([]const u8).init(allocator);
            const flag_list = &@field(@field(result, cmd.name), flag.long);
            flag_list.* = array_list;
            try flag_list.ensureTotalCapacity(argv.len);
        }
    }

    switch (p.cmds.len) {
        inline 0 => @compileError("at least one command must be provided"),
        inline 1 => if (p.cmds[0].desc.len > 0) {
            @compileError("a command with non-empty .desc implies the " ++
                "existence of several others, \n" ++
                "but there is only 1; use ParseParams.desc");
        },
        inline else => {},
    }

    inline for (p.cmds) |cmd| {
        inline for (cmd.flags, 0..) |flag, i| {
            inline for (cmd.flags[i + 1 ..]) |flag_cmp| {
                const duplicate_flag =
                    (flag.short != 0 and flag_cmp.short != 0) and
                    (flag.short == flag_cmp.short);

                if (duplicate_flag) @compileError(std.fmt.comptimePrint(
                    "flags '{s}' and '{s}' belong to the same command " ++
                        "and cannot share their short form '{c}'",
                    .{ flag.long, flag_cmp.long, flag.short },
                ));
            }
        }
    }

    const kinds = try allocator.alloc(ArgKind, argv.len - 1);
    defer allocator.free(kinds);
    procArgKindList(p, kinds, argv[1..]);

    inline for (p.cmds) |cmd| {
        const good_cmd = p.cmds.len == 1 or std.mem.eql(u8, argv[1], cmd.name);
        if (good_cmd) {
            _ = try procCmd(
                writer,
                err_writer,
                argv,
                p,
                cmd,
                kinds,
                result,
            ) orelse return null;
            break;
        }
    } else {
        const requested_help =
            std.mem.eql(u8, argv[1], "--" ++ help_flag.long) or
            std.mem.eql(u8, argv[1], "-" ++ .{help_flag.short});

        if (requested_help) {
            try printHelp(writer, argv[0], p, null);
            return null;
        }

        const requested_ver = std.mem.eql(u8, argv[1], "--" ++ ver_flag.long);

        if (requested_ver) {
            try printVer(writer, argv[0], p);
            return null;
        }

        switch (kinds[0]) {
            .cmd => {
                return errMsg(p.err_msg, Err.InvalidCmd, err_writer, argv[1]);
            },
            .pos => unreachable,
            .short, .long, .pos_marker => {
                return errMsg(p.err_msg, Err.MissingCmd, err_writer, &.{});
            },
        }
    }

    return result;
}

fn procCmd(
    writer: Writer,
    err_writer: Writer,
    argv: [][]const u8,
    comptime p: ParseParams,
    comptime cmd: Cmd,
    kinds: []ArgKind,
    result: *Cmd.listResultType(p.cmds),
) !?void {
    var cmd_res = &@field(result, cmd.name);

    cmd_res.invoked = true;

    var got_pos = false;
    var got_cmd = if (p.cmds.len == 1) true else false;

    var i: usize = 1;

    cmd_arg: while (i < argv.len) {
        const arg = argv[i];
        const arg_kind = kinds[i - 1];

        switch (arg_kind) {
            .cmd => {
                if (!got_cmd) got_cmd = true;
            },
            .pos => switch (cmd.kind) {
                inline .boolean => return errMsg(
                    p.err_msg,
                    Err.UnexpectedArg,
                    err_writer,
                    argv[i],
                ),
                inline .single_pos => {
                    if (got_pos) return errMsg(
                        p.err_msg,
                        Err.UnexpectedArg,
                        err_writer,
                        argv[i],
                    );

                    cmd_res.pos = arg;
                    got_pos = true;
                },
                inline .multi_pos => cmd_res.pos.appendAssumeCapacity(arg),
            },
            .pos_marker => {},
            .short => for (arg[1..], 1..) |short, short_i| {
                if (short == help_flag.short) {
                    try printHelp(writer, argv[0], p, cmd);
                    return null;
                }

                if (cmd.flags.len == 0) return errMsg(
                    p.err_msg,
                    Err.InvalidFlag,
                    err_writer,
                    argv[i],
                );

                match: inline for (cmd.flags) |flag| {
                    var flag_res = &@field(cmd_res, flag.long);
                    const matched = flag.short != 0 and flag.short == short;

                    if (matched) switch (flag.kind) {
                        inline .boolean => flag_res.* = true,
                        inline .single_pos => {
                            const no_pos = (short_i == arg.len - 1) and
                                (i == argv.len - 1 or kinds[i] != .pos);

                            if (no_pos) return errMsg(
                                p.err_msg,
                                Err.MissingArg,
                                err_writer,
                                &.{argv[i][short_i]},
                            );

                            // Format: -fval

                            if (short_i < arg.len - 1) {
                                flag_res.* = argv[i][short_i + 1 ..];
                                i += 1;
                                continue :cmd_arg;
                            }

                            // Format: -f val

                            i += 1;
                            flag_res.* = argv[i];
                            i += 1;
                            continue :cmd_arg;
                        },
                        inline .multi_pos => {
                            const no_pos = (short_i == arg.len - 1) and
                                (i == argv.len - 1 or kinds[i] != .pos);

                            if (no_pos) return errMsg(
                                p.err_msg,
                                Err.MissingArg,
                                err_writer,
                                &.{argv[i][short_i]},
                            );

                            if (short_i < arg.len - 1) {
                                flag_res.appendAssumeCapacity(
                                    argv[i][short_i + 1 ..],
                                );
                            }

                            i += 1;
                            while (i < argv.len and kinds[i - 1] == .pos) {
                                flag_res.appendAssumeCapacity(argv[i]);
                                i += 1;
                            } else continue :cmd_arg;
                        },
                    };
                    if (matched) break :match;
                } else return errMsg(
                    p.err_msg,
                    Err.InvalidFlag,
                    err_writer,
                    &.{argv[i][short_i]},
                );
            },
            .long => {
                const requested_help =
                    std.mem.eql(u8, arg[2..], help_flag.long);

                if (requested_help) {
                    try printHelp(writer, argv[0], p, cmd);
                    return null;
                }

                if (cmd.flags.len == 0) return errMsg(
                    p.err_msg,
                    Err.InvalidFlag,
                    err_writer,
                    argv[i],
                );

                match: inline for (cmd.flags) |flag| {
                    var flag_res = &@field(cmd_res, flag.long);

                    const equals_syntax =
                        (flag.kind != .boolean and
                        std.mem.startsWith(u8, arg[2..], flag.long) and
                        (arg[2..].len > flag.long.len + 1) and
                        (arg[2 + flag.long.len] == '='));

                    const matched_long = std.mem.eql(u8, arg[2..], flag.long);
                    const matched = equals_syntax or matched_long;

                    if (matched) switch (flag.kind) {
                        inline .boolean => {
                            flag_res.* = true;
                            i += 1;
                            continue :cmd_arg;
                        },
                        inline .single_pos => {
                            const no_pos_after_arg =
                                (i == argv.len - 1 or kinds[i] != .pos);

                            if (!equals_syntax and no_pos_after_arg) {
                                return errMsg(
                                    p.err_msg,
                                    Err.MissingArg,
                                    err_writer,
                                    argv[i],
                                );
                            }

                            if (equals_syntax) {
                                const start_offset =
                                    "--=".len + flag.long.len + "=".len;
                                flag_res.* = argv[i][start_offset..];
                            } else {
                                i += 1;
                                flag_res.* = argv[i];
                            }

                            i += 1;
                            continue :cmd_arg;
                        },
                        inline .multi_pos => {
                            const no_pos_after_arg =
                                (i == argv.len - 1 or kinds[i] != .pos);

                            if (!equals_syntax and no_pos_after_arg) {
                                return errMsg(
                                    p.err_msg,
                                    Err.MissingArg,
                                    err_writer,
                                    argv[i],
                                );
                            }

                            if (equals_syntax) {
                                const start_offset =
                                    "--=".len + flag.long.len + "=".len;
                                const pos = argv[i][start_offset..];
                                flag_res.appendAssumeCapacity(pos);
                            }

                            i += 1;
                            while (i < argv.len and kinds[i - 1] == .pos) {
                                flag_res.appendAssumeCapacity(argv[i]);
                                i += 1;
                            } else continue :cmd_arg;
                        },
                    };
                    if (matched) break :match;
                } else return errMsg(
                    p.err_msg,
                    Err.InvalidFlag,
                    err_writer,
                    argv[i],
                );
                i += 1;
                continue :cmd_arg;
            },
        }
        i += 1;
        continue :cmd_arg;
    } // :cmd_arg

    const no_pos = (cmd.kind == .single_pos) and (!got_pos or argv.len == 1);

    if (no_pos) {
        try printHelp(err_writer, argv[0], p, cmd);
        const cmd_i = if (p.cmds.len == 1) 0 else 1;
        return errMsg(p.err_msg, Err.MissingArg, err_writer, argv[cmd_i]);
    }

    inline for (cmd.flags) |flag| {
        const flag_res = &@field(cmd_res, flag.long);

        if (flag.required) switch (flag.kind) {
            inline .boolean => if (flag_res.* == false) {
                return errMsg(p.err_msg, Err.MissingFlag, err_writer, argv[0]);
            },
            inline .single_pos => if (flag_res.pos == null) {
                return errMsg(p.err_msg, Err.MissingFlag, err_writer, argv[0]);
            },
            inline .multi_pos => if (flag_res.items.len == 0) {
                return errMsg(p.err_msg, Err.MissingFlag, err_writer, argv[0]);
            },
        };
    }
}

const ArgKind = enum { cmd, pos, short, long, pos_marker };

fn procArgKindList(
    comptime p: ParseParams,
    kinds: []ArgKind,
    argv: []const []const u8,
) void {
    var only_pos = false;
    var got_cmd = if (p.cmds.len > 1) false else true;

    for (argv, 0..) |arg, i| {
        if (only_pos or arg.len == 0) {
            kinds[i] = .pos;
        } else if (arg.len == 1) {
            if (!got_cmd) kinds[i] = .cmd;
            kinds[i] = .pos;
        } else if (std.mem.eql(u8, arg, "--")) {
            kinds[i] = .pos_marker;
            only_pos = true;
        } else if (std.mem.startsWith(u8, arg, "--")) {
            kinds[i] = .long;
        } else if (arg[0] == '-') {
            kinds[i] = .short;
        } else if (!got_cmd) {
            got_cmd = true;
            kinds[i] = .cmd;
        } else kinds[i] = .pos;
    }
}

const indent = "  ";
const indent_required = "* ";

pub fn printHelp(
    writer: Writer,
    name: []const u8,
    comptime p: ParseParams,
    comptime cmd: ?Cmd,
) !void {
    if (cmd == null or p.cmds.len == 1) {
        if (p.desc.len > 0 or p.ver.len > 0) {
            _ = try writer.write(name);
            if (p.desc.len > 0) try writer.print(" - {s}", .{p.desc});
            if (p.ver.len > 0) try writer.print(" (version {s})", .{p.ver});
            try writer.writeByte('\n');
        }

        if (p.usage.len > 0) try writer.print(
            "\nUsage:\n{s}{s} {s}\n",
            .{ indent, name, p.usage },
        );

        if (p.cmds.len > 1) {
            _ = try writer.write("\nCommands:\n");
            inline for (p.cmds) |subcmd| {
                try printCmd(writer, subcmd);
            }
            _ = try writer.write("\nGeneral options:\n");
            try printFlag(writer, help_flag);
            if (p.ver.len > 0) try printFlag(writer, ver_flag);
        } else {
            _ = try writer.write("\nOptions:\n");

            const all_flags = p.cmds[0].flags ++ .{help_flag};
            inline for (all_flags) |flag| {
                try printFlag(writer, flag);
            }
        }
    } else {
        if (cmd.?.desc.len > 0) {
            try writer.print("{s} - {s}\n\n", .{ cmd.?.name, cmd.?.desc });
        }

        if (cmd.?.usage.len > 0) try writer.print(
            "Usage:\n{s}{s} {s} {s}\n\n",
            .{ indent, name, cmd.?.name, cmd.?.usage },
        );

        inline for (cmd.?.flags) |flag| {
            if (!flag.required) continue;
            try writer.print(
                "Options marked with '{c}' are required.\n\n",
                .{indent_required[0]},
            );
            break;
        }

        _ = try writer.write("Options:\n");

        const all_flags = cmd.?.flags ++ .{help_flag};
        inline for (all_flags) |flag| try printFlag(writer, flag);
    }

    try writer.writeByte('\n');
}

pub fn printVer(
    writer: Writer,
    name: []const u8,
    comptime p: ParseParams,
) !void {
    if (p.ver.len == 0) return;
    try writer.print("{s} (version {s})\n", .{ name, p.ver });
}

pub fn printFlag(writer: Writer, comptime flag: Flag) !void {
    const indent_str = if (flag.required) indent_required else indent;
    try writer.print("{s}", .{indent_str});

    if (flag.short != 0) {
        try writer.print("-{c}, ", .{flag.short});
    } else _ = try writer.write("    ");

    try writer.print("--{s}", .{flag.long});

    if (flag.usage.len > 0) try writer.print(" {s}", .{flag.usage});

    if (flag.desc.len > 0) try writer.print("\n\t{s}", .{flag.desc});

    try writer.writeByte('\n');
}

pub fn printCmd(writer: Writer, comptime cmd: Cmd) !void {
    try writer.print("{s}{s}", .{ indent, cmd.name });

    if (cmd.usage.len > 0) try writer.print(" {s}", .{cmd.usage});

    if (cmd.desc.len > 0) try writer.print("\n\t{s}", .{cmd.desc});

    try writer.writeByte('\n');
}

// ------
// This software is available under 2 licences. Choose whichever you prefer.
// ------
// ALTERNATIVE A - BSD-3-Clause (https://opensource.org/license/bsd-3-clause/)
// Copyright 2023 felix-u
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS
// IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ------
// ALTERNATIVE B - Public Domain (https://unlicense.org)
// This is free and unencumbered software released into the public domain.
// Anyone is free to copy, modify, publish, use, compile, sell, or distribute
// this software, either in source code form or as a compiled binary, for any
// purpose, commercial or non-commercial, and by any means.
// In jurisdictions that recognize copyright laws, the author or authors of
// this software dedicate any and all copyright interest in the software to the
// public domain. We make this dedication for the benefit of the public at
// large and to the detriment of our heirs and successors. We intend this
// dedication to be an overt act of relinquishment in perpetuity of all present
// and future rights to this software under copyright law.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// For more information, please refer to <http://unlicense.org/>
// ------
