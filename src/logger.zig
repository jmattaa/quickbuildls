const std = @import("std");
const ctime = @cImport({
    @cInclude("time.h");
});

pub const Logger = struct {
    logfn: []const u8,
    logfile: std.fs.File,

    pub fn init(logfn: []const u8) !Logger {
        const file = try std.fs.createFileAbsolute(
            logfn,
            .{
                .truncate = true,
                .mode = 0o600,
            },
        );

        return .{
            .logfn = logfn,
            .logfile = file,
        };
    }
    pub fn deinit(self: Logger) void {
        self.logfile.close();
    }

    pub fn write(
        self: Logger,
        allocator: std.mem.Allocator,
        comptime fmt: []const u8,
        args: anytype,
    ) !void {
        var now: ctime.time_t = undefined;
        _ = ctime.time(&now);
        const timeinfo = ctime.localtime(&now);
        var s = std.mem.span(ctime.asctime(timeinfo));
        s[s.len - 1] = ' '; // replace the \n with ' '

        try self.logfile.writeAll(s);
        try self.logfile.writeAll(" "); // add an extra space after timestamp
        const formatted = try std.fmt.allocPrint(allocator, fmt, args);
        defer allocator.free(formatted);
        try self.logfile.writeAll(formatted);
        try self.logfile.writeAll("\n");
    }

    pub fn writeRaw(self: Logger, data: []const u8) !void {
        try self.logfile.writeAll(data);
    }
};
