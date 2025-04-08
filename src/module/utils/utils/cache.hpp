#pragma once

#include <cstdint>
#include <array>
#include <optional>

/// A caching structure that keeps up to \p capacity records
/// Optimized for low capacity (<= 16), but big items. The items never get moved around.
template <typename Key_, typename Value_, size_t capacity_>
class Cache {
    static_assert(capacity_ <= 16, "This Cache implementation is intended only for small amount of records");

public:
    static constexpr size_t capacity = capacity_;
    using Key = Key_;
    using Value = Value_;

public:
    /// \returns a record for \p key, if it is in the cache. nullptr otherwise
    Value *find(const Key &key) {
        if (auto rec = find_rec(key)) {
            // find_rec calls record_access, no need to do it here again
            return &rec->value;

        } else {
            return nullptr;
        }
    }

    /// Finds or creates a record for \p key.
    /// This can result in other records being removed.
    /// \returns the existing or newly created record
    /// \param existed if set, value of this variable will be set to true if the record was already in the cache, false if it was newly created
    Value &get(const Key &key, bool *existed = nullptr) {
        OptionalRecord *rec = find_rec(key);

        if (existed) {
            *existed = (rec != nullptr);
        }

        if (!rec) {
            rec = get_slot();
            rec->emplace();
            (*rec)->key = key;
            record_access(**rec);
        }

        return (**rec).value;
    }

    /// Removes the record for \p key from the cache.
    /// \returns true if the record was in the cache and was removed
    bool invalidate(const Key &key) {
        for (auto &rec : records_) {
            if (rec.has_value() && rec->key == key) {
                rec.reset();
                return true;
            }
        }

        return false;
    }

    /// Removes all records
    void clear() {
        for (auto &rec : records_) {
            rec.reset();
        }

        // Reset to 1 - access ix 0 is dedicated for items not yet accessed at all
        access_counter_ = 1;
    }

private:
    using AccessIx = uint16_t;

    struct Record {
        Value value;
        Key key;
        AccessIx last_access = 0;
    };

    /// Make records optional, to have proper value lifetime
    using OptionalRecord = std::optional<Record>;

    std::array<OptionalRecord, capacity> records_;

    /// Increases with each access
    AccessIx access_counter_ = 1;

private:
    /// \returns a record for \p key, if it is in the cache. nullptr otherwise
    OptionalRecord *find_rec(const Key &key) {
        for (auto &rec : records_) {
            if (rec.has_value() && rec->key == key) {
                record_access(*rec);
                return &rec;
            }
        }

        return nullptr;
    }

    void record_access(Record &rec) {
        // Reduce AccessIx overflows by excluding consecutive accesses to the same record
        if (rec.last_access == access_counter_) {
            return;
        }

        access_counter_++;

        // The access counter has overflown - reset all the access counters to prevent weird behavior
        // Yes, we are losing the usage data here, but whatevs, the overflow almost never happens
        if (access_counter_ == 0) {
            // The initial value for access_counter is actually 1
            access_counter_ = 1;

            for (auto &rec : records_) {
                if (rec.has_value()) {
                    rec->last_access = 0;
                }
            }
        }

        rec.last_access = access_counter_;
    }

    /// Returns either an unused cache slot, or the one most suitable for recycling
    OptionalRecord *get_slot() {
        OptionalRecord *oldest_rec = nullptr;

        for (auto &rec : records_) {
            if (!rec.has_value()) {
                return &rec; // Empty record -> best candidate

            } else if (!oldest_rec || (*rec).last_access < (**oldest_rec).last_access) {
                oldest_rec = &rec;
            }
        }

        return oldest_rec;
    }
};
