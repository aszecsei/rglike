/**
 * @file math.hpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */

#pragma once

#include <array>
#include <cmath>
#include <fmt/format.h>

namespace rglike {
    class Vec2 {
    private:
        std::array<int, 2> m_e;

    public:
        static constexpr auto Zero() -> Vec2 { return Vec2{0, 0}; }

        static constexpr auto One() -> Vec2 { return Vec2{1, 1}; }

        static constexpr auto Up() -> Vec2 { return Vec2{0, -1}; }

        static constexpr auto Left() -> Vec2 { return Vec2{-1, 0}; }

        static constexpr auto Down() -> Vec2 { return Vec2{0, 1}; }

        static constexpr auto Right() -> Vec2 { return Vec2{1, 0}; }

        constexpr Vec2(int x, int y)
            : m_e{x, y} { }

        inline auto X() -> int& { return m_e[0]; }

        inline auto Y() -> int& { return m_e[1]; }

        [[nodiscard]] constexpr auto X() const -> int { return m_e[0]; }

        [[nodiscard]] constexpr auto Y() const -> int { return m_e[1]; }

        constexpr auto operator-() const -> Vec2 { return Vec2{-m_e[0], -m_e[1]}; }

        constexpr auto operator[](int idx) const -> int { return m_e[idx]; }

        constexpr auto operator[](int idx) -> int& { return m_e[idx]; }

        constexpr auto operator+=(const Vec2& vec) -> Vec2& {
            m_e[0] += vec.m_e[0];
            m_e[1] += vec.m_e[1];
            return *this;
        }

        constexpr auto operator-=(const Vec2& vec) -> Vec2& {
            m_e[0] -= vec.m_e[0];
            m_e[1] -= vec.m_e[1];
            return *this;
        }

        constexpr auto operator*=(const int scalar) -> Vec2& {
            m_e[0] *= scalar;
            m_e[1] *= scalar;
            return *this;
        }

        constexpr auto operator/=(const int scalar) -> Vec2& {
            m_e[0] /= scalar;
            m_e[1] /= scalar;
            return *this;
        }

        [[nodiscard]] constexpr auto LengthSquared() const -> double {
            return m_e[0] * m_e[0] + m_e[1] * m_e[1];
        }

        [[nodiscard]] inline auto Length() const -> double { return std::sqrt(LengthSquared()); }
    };

    constexpr auto operator+(Vec2 lhs, const Vec2& rhs) -> Vec2 {
        lhs += rhs;
        return lhs;
    }

    constexpr auto operator-(Vec2 lhs, const Vec2& rhs) -> Vec2 {
        lhs -= rhs;
        return lhs;
    }

    constexpr auto operator*(Vec2 lhs, const int& rhs) -> Vec2 {
        lhs *= rhs;
        return lhs;
    }

    constexpr auto operator/(Vec2 lhs, const int& rhs) -> Vec2 {
        lhs /= rhs;
        return lhs;
    }

    constexpr auto operator==(const Vec2& lhs, const Vec2& rhs) -> bool {
        return lhs.X() == rhs.X() && lhs.Y() == rhs.Y();
    }

    constexpr auto operator!=(const Vec2& lhs, const Vec2& rhs) -> bool { return !(lhs == rhs); }
} // namespace rglike

template<> struct fmt::formatter<rglike::Vec2> : fmt::formatter<int> {
    template<typename FormatContext> auto format(const rglike::Vec2& vec, FormatContext& ctx) {
        auto&& out = ctx.out();
        fmt::format_to(out, "(");
        fmt::formatter<int>::format(vec.X(), ctx);
        fmt::format_to(out, ", ");
        fmt::formatter<int>::format(vec.Y(), ctx);
        return fmt::format_to(ctx.out(), ")");
    }
};