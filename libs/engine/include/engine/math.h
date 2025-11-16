#pragma once

namespace engine {

// 2D vector for position and direction
struct Vector2 {
    int x, y;

    Vector2() : x(0), y(0) {}
    Vector2(int x, int y) : x(x), y(y) {}

    // Arithmetic operators
    Vector2 operator+(const Vector2& other) const {
        return {x + other.x, y + other.y};
    }

    Vector2 operator-(const Vector2& other) const {
        return {x - other.x, y - other.y};
    }

    Vector2 operator*(int scalar) const {
        return {x * scalar, y * scalar};
    }

    Vector2 operator/(int scalar) const {
        return {x / scalar, y / scalar};
    }

    // Compound assignment operators
    Vector2& operator+=(const Vector2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vector2& operator-=(const Vector2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    // Comparison operators
    bool operator==(const Vector2& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Vector2& other) const {
        return !(*this == other);
    }

    // Manhattan distance
    [[nodiscard]] int manhattan_distance(const Vector2& other) const {
        return std::abs(x - other.x) + std::abs(y - other.y);
    }

    // Squared Euclidean distance (avoids sqrt for performance)
    [[nodiscard]] int distance_squared(const Vector2& other) const {
        int dx = x - other.x;
        int dy = y - other.y;
        return dx * dx + dy * dy;
    }
};

// Rectangle representing an axis-aligned bounding box
struct Rect {
    int x, y, w, h;

    Rect() : x(0), y(0), w(0), h(0) {}
    Rect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}

    // Get the center position of the rectangle
    [[nodiscard]] Vector2 center() const {
        return {x + w / 2, y + h / 2};
    }

    // Get corners
    [[nodiscard]] Vector2 top_left() const {
        return {x, y};
    }

    [[nodiscard]] Vector2 top_right() const {
        return {x + w - 1, y};
    }

    [[nodiscard]] Vector2 bottom_left() const {
        return {x, y + h - 1};
    }

    [[nodiscard]] Vector2 bottom_right() const {
        return {x + w - 1, y + h - 1};
    }

    // Check if a point is inside the rectangle
    [[nodiscard]] bool contains(const Vector2& point) const {
        return point.x >= x && point.x < x + w &&
               point.y >= y && point.y < y + h;
    }

    [[nodiscard]] bool contains(int px, int py) const {
        return contains(Vector2(px, py));
    }

    // Check if two rectangles overlap
    [[nodiscard]] bool intersects(const Rect& other) const {
        return x < other.x + other.w &&
               x + w > other.x &&
               y < other.y + other.h &&
               y + h > other.y;
    }

    // Get the area
    [[nodiscard]] int area() const {
        return w * h;
    }

    // Comparison operators
    bool operator==(const Rect& other) const {
        return x == other.x && y == other.y && w == other.w && h == other.h;
    }

    bool operator!=(const Rect& other) const {
        return !(*this == other);
    }
};

} // namespace engine
