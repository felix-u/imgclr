const std = @import("std");

const zstbi = @import("libs/zstbi/build.zig");

pub fn build(b: *std.Build) !void {

    const exe_name = "imgclr";
    const exe_version = "0.2-dev";

    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const zig_clap_module = b.createModule(.{
        .source_file = .{ .path = "libs/zig-clap/clap.zig" }
    });

    const zstbi_pkg = zstbi.Package.build(b, .{}, optimize, .{});


    const exe = b.addExecutable(.{
        .name = exe_name,
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });
    exe.addModule("clap", zig_clap_module);
    exe.addModule("zstbi", zstbi_pkg.zstbi);
    zstbi_pkg.link(exe);
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
    debug_exe.addModule("clap", zig_clap_module);
    debug_exe.addModule("zstbi", zstbi_pkg.zstbi);
    zstbi_pkg.link(debug_exe);
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
    release_exe.addModule("clap", zig_clap_module);
    release_exe.addModule("zstbi", zstbi_pkg.zstbi);
    zstbi_pkg.link(release_exe);
    release_exe.addIncludePath("src/");
    release_exe.addIncludePath("libs/");
    release_exe.strip = true;
    release_exe.linkage = .static;
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
            cross_exe.strip = true;
            cross_exe.linkage = .static;
            cross_exe.addModule("clap", zig_clap_module);
            cross_exe.addModule("zstbi", zstbi_pkg.zstbi);
            zstbi_pkg.link(cross_exe);
            cross_exe.addIncludePath("src/");
            cross_exe.addIncludePath("libs/");
            cross_step.dependOn(&b.addInstallArtifact(cross_exe).step);
        }
    }

}
