#pragma once
namespace RNS { namespace Utilities {
// Preserve the outer transport critical section across synchronous re-entry.
// This is cooperative state restoration, not a thread mutex.
class ScopedFlag {
    bool& flag;
    const bool previous;
public:
    explicit ScopedFlag(bool& value) noexcept : flag(value), previous(value) { flag = true; }
    ~ScopedFlag() noexcept { flag = previous; }
    ScopedFlag(const ScopedFlag&) = delete;
    ScopedFlag& operator=(const ScopedFlag&) = delete;
};
} }
