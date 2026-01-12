const std = @import("std");
const Allocator = std.mem.Allocator;

pub const LineLength = 80;
pub const SeparatorChars = " .,\"\n";

/// Entry in the hash table
pub const Entry = struct {
    key: []const u8,
    value: []const u8,
    next: ?*Entry,
};

/// Hash table implementation for storing responses
pub const HashTable = struct {
    size: usize,
    table: []?*Entry,
    allocator: Allocator,

    /// Create a new hash table
    pub fn create(allocator: Allocator, size: usize) !*HashTable {
        if (size < 1) return error.InvalidSize;

        const ht = try allocator.create(HashTable);
        ht.table = try allocator.alloc(?*Entry, size);
        @memset(ht.table, null);
        ht.size = size;
        ht.allocator = allocator;

        return ht;
    }

    /// Hash a string key
    fn hash(self: *const HashTable, key: []const u8) usize {
        var hashval: u64 = 0;
        for (key) |char| {
            hashval = (hashval << 8) +% @as(u64, char);
        }
        return hashval % self.size;
    }

    /// Insert or update a key-value pair
    pub fn set(self: *HashTable, key: []const u8, value: []const u8) !void {
        const bin = self.hash(key);
        var next = self.table[bin];
        var last: ?*Entry = null;

        // Find the correct position in the chain
        while (next) |current| {
            const cmp = std.mem.order(u8, current.key, key);
            if (cmp == .gt) {
                break;
            }

            if (cmp == .eq) {
                // Key exists, update value
                self.allocator.free(current.value);
                current.value = try self.allocator.dupe(u8, value);
                return;
            }

            last = current;
            next = current.next;
        }

        // Key doesn't exist, create new entry
        const newpair = try self.allocator.create(Entry);
        newpair.key = try self.allocator.dupe(u8, key);
        newpair.value = try self.allocator.dupe(u8, value);
        newpair.next = next;

        if (last) |l| {
            l.next = newpair;
        } else {
            self.table[bin] = newpair;
        }
    }

    /// Retrieve a value by key
    pub fn get(self: *const HashTable, key: []const u8) ?[]const u8 {
        const bin = self.hash(key);
        var pair = self.table[bin];

        while (pair) |current| {
            const cmp = std.mem.order(u8, current.key, key);
            if (cmp == .gt) {
                break;
            }

            if (cmp == .eq) {
                return current.value;
            }

            pair = current.next;
        }

        return null;
    }

    /// Cleanup hash table resources
    pub fn destroy(self: *HashTable) void {
        for (self.table) |entry_opt| {
            var entry = entry_opt;
            while (entry) |current| {
                const next = current.next;
                self.allocator.free(current.key);
                self.allocator.free(current.value);
                self.allocator.destroy(current);
                entry = next;
            }
        }
        self.allocator.free(self.table);
        self.allocator.destroy(self);
    }
};

test "create_hashtable" {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    const ht = try HashTable.create(allocator, 100);
    defer ht.destroy();

    try std.testing.expectEqual(ht.size, 100);
}

test "set_and_get" {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var ht = try HashTable.create(allocator, 100);
    defer ht.destroy();

    try ht.set("key", "value");
    const result = ht.get("key");

    try std.testing.expect(result != null);
    try std.testing.expectEqualSlices(u8, result.?, "value");
}

test "get_nonexistent_key" {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var ht = try HashTable.create(allocator, 100);
    defer ht.destroy();

    const result = ht.get("nonexistent");

    try std.testing.expectEqual(result, null);
}

test "update_existing_key" {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var ht = try HashTable.create(allocator, 100);
    defer ht.destroy();

    try ht.set("key", "value1");
    try ht.set("key", "value2");

    const result = ht.get("key");
    try std.testing.expect(result != null);
    try std.testing.expectEqualSlices(u8, result.?, "value2");
}

test "multiple_entries" {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var ht = try HashTable.create(allocator, 100);
    defer ht.destroy();

    try ht.set("hi", "hello");
    try ht.set("hey", "hello");
    try ht.set("python", "Yo, I love Python");

    try std.testing.expect(ht.get("hi") != null);
    try std.testing.expectEqualSlices(u8, ht.get("hi").?, "hello");

    try std.testing.expect(ht.get("python") != null);
    try std.testing.expectEqualSlices(u8, ht.get("python").?, "Yo, I love Python");

    try std.testing.expect(ht.get("missing") == null);
}
