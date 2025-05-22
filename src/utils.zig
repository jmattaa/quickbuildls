const std = @import("std");

pub const VERSION = "0.0.1";

pub const CutError = error{NotFound};

// so we get something like the go bytes.Cut function
pub fn cut(arr: []const u8, sep: []const u8) CutError![2][]const u8 {
    const index = std.mem.indexOf(u8, arr, sep);
    if (index) |i| {
        return .{
            arr[0..i],
            arr[i + sep.len ..],
        };
    } else {
        return CutError.NotFound;
    }
}
