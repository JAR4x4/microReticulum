#pragma once
#include <cstddef>
namespace RNS { namespace Utilities {
// Allocation-free reservation for cooperative, possibly nested operations.
class ScopedCounter {
    size_t& count;
public:
    explicit ScopedCounter(size_t& value) noexcept : count(value) { ++count; }
    ~ScopedCounter() noexcept { --count; }
    ScopedCounter(const ScopedCounter&) = delete;
    ScopedCounter& operator=(const ScopedCounter&) = delete;
};
} }
