#pragma once
#include <cmath>
#include <limits>
#include <stdexcept>

template <typename T>
class Vector2
{
public:
    T X, Y;

    Vector2(T x, T y) : X(x), Y(y) {}

    Vector2<T> operator+(const Vector2<T> &other) const
    {
        return Vector2<T>(X + other.X, Y + other.Y);
    }

    Vector2<T> operator-(const Vector2<T> &other) const
    {
        return Vector2<T>(X - other.X, Y - other.Y);
    }

    Vector2<T> operator*(T scalar) const
    {
        return Vector2<T>(X * scalar, Y * scalar);
    }

    Vector2<T> operator/(T scalar) const
    {
        if (scalar != 0)
            return Vector2<T>(X / scalar, Y / scalar);
        else
            throw std::runtime_error("Division by zero");
    }

    bool operator==(const Vector2<T> &other) const
    {
        const T epsilon = std::numeric_limits<T>::epsilon();
        return std::abs(X - other.X) <= epsilon && std::abs(Y - other.Y) <= epsilon;
    }

    T Length() const
    {
        return std::sqrt(X * X + Y * Y);
    }

    Vector2<T> Normalized() const
    {
        T length = Length();
        if (length != 0)
            return Vector2<T>(X / length, Y / length);
        else
            return Vector2<T>(0, 0);
    }
};

using Vector2F = Vector2<float>;
using Vector2Int = Vector2<int>;