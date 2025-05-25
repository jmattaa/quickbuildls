const std = @import("std");

/// The state of the document, there is only going to be one document we have
/// to keep track of. Because we don't need any cross file completions and so on
pub const State = struct {
    document: ?[]u8,
    version: u32,

    pub fn init() !State {
        return State{
            .document = null,
            .version = 0,
        };
    }

    pub fn setDocument(
        self: *State,
        allocator: std.mem.Allocator,
        data: []const u8,
    ) !void {
        // free old document
        if (self.document) |doc| {
            allocator.free(doc);
        }

        // my dumbass thought that dupe dosent allocate memory
        self.document = try allocator.dupe(u8, data);
    }

    pub fn deinit(self: *State, allocator: std.mem.Allocator) void {
        if (self.document) |doc| {
            allocator.free(doc);
            self.document = null;
        }
    }
};
