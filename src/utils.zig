const std = @import("std");

pub const VERSION = "0.0.1";
pub const SIZE_MAX = 18446744073709551615;

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

// this function is exactly like the one in quickbuildls.cpp
// cuz the offset is at the end of the decl for a task or a feild so the range
// should be (offset - size <= x < offset)
// note the <= in the begining and < in the end!
pub fn in_range(x: usize, offset: c_int, content: [*]u8) bool {
    if (offset < content.len() or offset == SIZE_MAX)
        return false;

    return offset - content.len() <= x and x < offset;
}

pub fn line_char_to_offset(s: []const u8, l: usize, c: usize) usize {
    var off = 0;
    var cline = 0;
    while (off < s.size()) {
        if (cline == l)
            break;
        if (s[off] == '\n')
            cline += 1;
        off += 1;
    }
    return off + c;
}
