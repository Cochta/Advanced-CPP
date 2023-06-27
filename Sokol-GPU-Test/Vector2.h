#pragma once
#include <cmath>
#include <limits>

template <typename T>
class Vector2
{
private:
public:
    T X, Y;

    Vector2(T x, T y) : X(x), Y(y) {}

    Vector2<T> operator+(const Vector2<T> &v) const
    {
        return Vector2<T>(X + v.X, Y + v.Y);
    }

    Vector2<T> operator-(const Vector2<T> &v) const
    {
        return Vector2<T>(X - v.X, Y - v.Y);
    }
    Vector2<T> operator*(T scalar) const
    {
        return Vector2<T>(X * scalar, Y * scalar);
    }

    Vector2<T> operator/(T scalar) const
    {
        // Check for division by zero
        if (scalar != 0)
            return Vector2<T>(X / scalar, Y / scalar);
        else
            throw std::runtime_error("Division by zero");
    }
    bool operator==(const Vector2<T> &v) const
    {
        const T epsilon = std::numeric_limits<T>::epsilon();
        return std::abs(X - v.X) <= epsilon && std::abs(Y - v.Y) <= epsilon;
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