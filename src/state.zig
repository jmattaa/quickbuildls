const std = @import("std");
const cstate = @cImport({
    @cInclude("state.h");
});

/// The state of the document, there is only going to be one document we have
/// to keep track of. Because we don't need any cross file completions and so on
pub const State = struct {
    document: ?[]u8,
    version: u32,
    cstate: ?*cstate.qls_state,

    pub fn init() !State {
        const s = State{
            .document = null,
            .version = 0,
            .cstate = cstate.qls_state_init(null),
        };

        return s;
    }

    pub fn setDocument(
        s: *State,
        allocator: std.mem.Allocator,
        data: []const u8,
    ) !void {
        // free old document
        if (s.document) |doc| {
            allocator.free(doc);
        }

        // my dumbass thought that dupe dosent allocate memory
        s.document = try allocator.dupe(u8, data);
        if (s.document) |doc| {
            cstate.qls_state_update(s.cstate, doc.ptr);
        }
    }

    pub fn deinit(s: *State, allocator: std.mem.Allocator) void {
        if (s.document) |doc| {
            allocator.free(doc);
            s.document = null;
        }
        if (s.cstate) |state| {
            cstate.qls_state_free(state);
            s.cstate = null;
        }
    }
};
