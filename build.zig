const builtin = @import("builtin");
const std = @import("std");

pub fn build(b: *std.Build) !void {
    const optimize = b.standardOptimizeOption(.{});
    const target = b.standardTargetOptions(.{});

    const exe = b.addExecutable(.{
        .name = "imgclr",
        .target = target,
        .optimize = optimize,
        .root_source_file = .{ .path = "src/main.zig" },
    });

    exe.linkLibC();
    exe.addCSourceFile(.{
        .file = .{ .path = "src/stbi.c" },
        .flags = &.{ "-O3", "-s" },
    });
    exe.addIncludePath(.{ .path = "src/" });

    b.installArtifact(exe);
}
