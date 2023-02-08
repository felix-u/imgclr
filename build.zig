const std = @import("std");

pub fn build(b: *std.Build) !void {

    const exe_name = "imgclr";
    const exe_version = "0.2-dev";

    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const zig_clap_module = b.createModule(.{
        .source_file = .{ .path = "zig-clap/clap.zig" }
    });

    // const cc_shared_flags = [_][]const u8 {
    //     "-std=c99",
    //     "-Wall",
    //     "-Wextra",
    //     "-pedantic",
    //     "-Wshadow",
    //     "-Wstrict-overflow",
    //     "-Wstrict-aliasing",
    //     // libs
    //     // "-lm",
    // };
    // const cc_debug_flags = cc_shared_flags ++ .{
    //     "-g",
    //     "-Og",
    //     "-ggdb",
    // };
    // const cc_release_flags = cc_shared_flags ++ .{
    //     "-O3",
    //     "-s",
    //     "-static",
    //     "-march=native",
    // };


    const exe = b.addExecutable(.{
        .name = exe_name,
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });
    // exe.addCSourceFile("src/main.c", &cc_shared_flags);
    exe.linkLibC();
    exe.addModule("clap", zig_clap_module);
    exe.addIncludePath("src/");
    exe.addIncludePath("libs/");
    exe.install();


    const debug_step = b.step("debug", "build debug exe");
    const debug_exe = b.addExecutable(.{
        .name = exe_name,
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = .Debug,
    });
    // debug_exe.addCSourceFile("src/main.c", &cc_debug_flags);
    debug_exe.linkLibC();
    debug_exe.addModule("clap", zig_clap_module);
    debug_exe.addIncludePath("src/");
    debug_exe.addIncludePath("libs/");
    debug_step.dependOn(&b.addInstallArtifact(debug_exe).step);

    const run_cmd = debug_exe.run();
    run_cmd.step.dependOn(debug_step);
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
    const run_step = b.step("run", "run the debug build");
    run_step.dependOn(&run_cmd.step);


    const release_step = b.step("release", "build release exe");
    const release_exe = b.addExecutable(.{
        .name = exe_name,
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = .ReleaseFast,
    });
    // release_exe.addCSourceFile("src/main.c", &cc_release_flags);
    release_exe.linkLibC();
    release_exe.addModule("clap", zig_clap_module);
    release_exe.addIncludePath("src/");
    release_exe.addIncludePath("libs/");
    release_exe.disable_sanitize_c = true;
    release_exe.strip = true;
    release_step.dependOn(&b.addInstallArtifact(release_exe).step);


    const cross_step = b.step("cross", "cross-compile for all targets");
    const target_architectures = [_][]const u8 {
        "x86_64", "aarch64",
    };
    const target_OSes = [_][]const u8 {
        "linux", "macos", "windows",
    };
    inline for (target_architectures) |target_arch| {
        inline for (target_OSes) |target_OS| {
            const triple = target_arch ++ "-" ++ target_OS;
            const cross_target = std.zig.CrossTarget.parse(.{ .arch_os_abi = triple }) catch unreachable;
            const cross_exe = b.addExecutable(.{
                .name = b.fmt("{s}-v{s}-{s}", .{exe_name, exe_version, triple}),
                .root_source_file = .{ .path = "src/main.zig" },
                .target = cross_target,
                .optimize = .ReleaseSafe,
            });
            // cross_exe.addCSourceFile("src/main.c", &(cc_shared_flags ++ .{ "-static" }));
            cross_exe.disable_sanitize_c = true;
            cross_exe.strip = true;
            cross_exe.linkLibC();
            cross_exe.addModule("clap", zig_clap_module);
            cross_exe.addIncludePath("src/");
            cross_exe.addIncludePath("libs/");
            cross_step.dependOn(&b.addInstallArtifact(cross_exe).step);
        }
    }

}
