#pragma once

#include <cstdint>
#include <array>
#include <limits>

namespace engine {

// WELL512 (Well Equidistributed Long-period Linear) PRNG
// Fast, high-quality random number generator with good statistical properties
class WELL512 {
public:
    using result_type = uint32_t;

    // Constructor with seed
    explicit WELL512(uint32_t seed = 5489u) {
        init(seed);
    }

    // Initialize state with seed
    void seed(uint32_t s) {
        init(s);
    }

    // Generate next random number
    uint32_t operator()() {
        uint32_t a = state_[index_];
        uint32_t c = state_[(index_ + 13) & 15];
        uint32_t b = a ^ c ^ (a << 16) ^ (c << 15);
        c = state_[(index_ + 9) & 15];
        c ^= (c >> 11);
        a = state_[index_] = b ^ c;
        uint32_t d = a ^ ((a << 5) & 0xDA442D24u);
        index_ = (index_ + 15) & 15;
        a = state_[index_];
        state_[index_] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
        return state_[index_];
    }

    // Static constexpr members required for std::uniform_int_distribution compatibility
    static constexpr uint32_t min() { return 0; }
    static constexpr uint32_t max() { return std::numeric_limits<uint32_t>::max(); }

    // Generate random number in range [0, 1)
    double uniform() {
        return static_cast<double>((*this)()) / static_cast<double>(max() + 1.0);
    }

    // Generate random integer in range [min_val, max_val]
    int range(int min_val, int max_val) {
        return min_val + static_cast<int>((*this)() % (max_val - min_val + 1));
    }

private:
    std::array<uint32_t, 16> state_;
    uint32_t index_ = 0;

    void init(uint32_t seed) {
        index_ = 0;
        state_[0] = seed;

        // Use a simple LCG to initialize the rest of the state
        for (int i = 1; i < 16; ++i) {
            state_[i] = (1812433253u * (state_[i - 1] ^ (state_[i - 1] >> 30)) + i);
        }
    }
};

} // namespace engine
