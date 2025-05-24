const std = @import("std");

/// The state of the document, there is only going to be one document we have
/// to keep track of. Because we don't need any cross file completions and so on
pub const State = struct {
    document: []u8,
    version: u32,

    pub fn init() !State {
        return State{
            .document = &[_]u8{},
            .version = 0,
        };
    }

    pub fn setDocument(
        self: *State,
        allocator: std.mem.Allocator,
        data: []const u8,
    ) !void {
        // free old document
        if (self.document.len != 0) {
            allocator.free(self.document);
        }

        self.document = try allocator.alloc(u8, data.len);
        self.document = try allocator.dupe(u8, data);
    }

    pub fn deinit(self: *State, allocator: std.mem.Allocator) void {
        if (self.document.len != 0) {
            allocator.free(self.document);
            self.document = &[_]u8{};
        }
    }
};
