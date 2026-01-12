const std = @import("std");
const chatbot = @import("chatbot.zig");

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    // Zig 0.15 I/O: explicit buffer management
    var stdout_buf: [4096]u8 = undefined;
    var stdout = std.fs.File.stdout().writer(&stdout_buf);

    var stdin_buf: [4096]u8 = undefined;
    var stdin = std.fs.File.stdin().reader(&stdin_buf);

    try stdout.interface.print("$ Chatbot v1.0.0!\n", .{});
    try stdout.interface.flush();

    // Create hash table
    var ht = try chatbot.HashTable.create(allocator, 65536);
    defer ht.destroy();

    // Populate responses
    try ht.set("hi", "hello");
    try ht.set("hey", "hello");
    try ht.set("hear", "What you heard is right");
    try ht.set("python", "Yo, I love Python");
    try ht.set("light", "I like light");
    try ht.set("What", "It is clear, ain't it?");

    while (true) {
        try stdout.interface.print("\n$ (user) ", .{});
        try stdout.interface.flush();

        // Read line using Zig 0.15 delimiter API
        // Use takeDelimiter which returns null on EOF with empty remaining
        const line = stdin.interface.takeDelimiter('\n') catch |err| {
            switch (err) {
                error.StreamTooLong => {
                    // Line too long, skip it
                    continue;
                },
                else => return err,
            }
        };

        if (line == null) break;

        const trimmed = std.mem.trim(u8, line.?, " \t\r\n");
        if (trimmed.len == 0) continue; // Empty line, keep going

        var word_iter = std.mem.tokenizeAny(u8, trimmed, chatbot.SeparatorChars);

        while (word_iter.next()) |word| {
            const lower_word = try allocator.alloc(u8, word.len);
            defer allocator.free(lower_word);

            for (word, 0..) |ch, i| {
                lower_word[i] = std.ascii.toLower(ch);
            }

            if (std.mem.eql(u8, lower_word, "exit")) {
                return;
            }

            if (ht.get(lower_word)) |response| {
                try stdout.interface.print("\n$ (chatbot) {s}\n", .{response});
            } else {
                try stdout.interface.print("\n$ (chatbot) {s}\n", .{"Sorry, I don't know what to say about that"});
            }
            try stdout.interface.flush();
        }
    }
}
