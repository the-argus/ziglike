const std = @import("std");
const builtin = @import("builtin");
const zcc = @import("compile_commands");

const release_flags = &[_][]const u8{
    "-DNDEBUG",
    "-std=c++17",
};

const debug_flags = &[_][]const u8{
    "-g",
    "-std=c++17",
    "-DFMT_EXCEPTIONS=1",
};

// effectively universal flags since the only thing we compile is tests
const testing_flags = &[_][]const u8{
    "-DZIGLIKE_HEADER_TESTING",
    "-DZIGLIKE_USE_FMT",
    "-I./tests/",
    "-I./include/",
};

const test_source_files = &[_][]const u8{
    "opt/opt.cpp",
    "res/res.cpp",
    "slice/slice.cpp",
};

pub fn build(b: *std.Build) !void {
    // options
    const target = b.standardTargetOptions(.{});
    const mode = b.standardOptimizeOption(.{});

    var flags = std.ArrayList([]const u8).init(b.allocator);
    defer flags.deinit();
    try flags.appendSlice(if (mode == .Debug) debug_flags else release_flags);
    try flags.appendSlice(testing_flags);

    var tests = std.ArrayList(*std.Build.Step.Compile).init(b.allocator);
    defer tests.deinit();

    // actual public installation step
    b.installDirectory(.{
        .source_dir = .{ .path = "include/ziglike/" },
        .install_dir = .header,
        .install_subdir = "ziglike/",
    });

    const fmt = b.dependency("fmt", .{});
    const fmt_include_path = b.pathJoin(&.{ fmt.builder.install_path, "include" });
    try flags.append(b.fmt("-I{s}", .{fmt_include_path}));

    const flags_owned = flags.toOwnedSlice() catch @panic("OOM");

    for (test_source_files) |source_file| {
        var test_exe = b.addExecutable(.{
            .name = std.fs.path.stem(source_file),
            .optimize = mode,
            .target = target,
        });
        test_exe.addCSourceFile(.{
            .file = .{ .path = b.pathJoin(&.{ "tests", source_file }) },
            .flags = flags_owned,
        });
        test_exe.linkLibCpp();
        test_exe.step.dependOn(fmt.builder.getInstallStep());
        try tests.append(test_exe);
    }

    const run_tests_step = b.step("run_tests", "Compile and run all the tests");
    const install_tests_step = b.step("install_tests", "Install all the tests but don't run them");
    for (tests.items) |test_exe| {
        const test_install = b.addInstallArtifact(test_exe, .{});
        install_tests_step.dependOn(&test_install.step);

        const test_run = b.addRunArtifact(test_exe);
        if (b.args) |args| {
            test_run.addArgs(args);
        }
        run_tests_step.dependOn(&test_run.step);
    }

    zcc.createStep(b, "cdb", try tests.toOwnedSlice());
}
