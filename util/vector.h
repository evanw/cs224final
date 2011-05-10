#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>
#include <stdlib.h>
#include <iostream>

#define M_2PI (2 * M_PI)

inline float frand() { return (float)rand() / (float)RAND_MAX; }
inline float min(float a, float b) { return a < b ? a : b; }
inline float max(float a, float b) { return a > b ? a : b; }

class Vector2
{
public:
    union
    {
        struct { float x, y; };
        float xy[3];
    };

    Vector2() : x(0), y(0) {}
    Vector2(float _x, float _y) : x(_x), y(_y) {}
    Vector2(const Vector2 &vec) : x(vec.x), y(vec.y) {}

    Vector2 operator - () const { return Vector2(-x, -y); }

    Vector2 operator + (const Vector2 &vec) const { return Vector2(x + vec.x, y + vec.y); }
    Vector2 operator - (const Vector2 &vec) const { return Vector2(x - vec.x, y - vec.y); }
    Vector2 operator * (const Vector2 &vec) const { return Vector2(x * vec.x, y * vec.y); }
    Vector2 operator / (const Vector2 &vec) const { return Vector2(x / vec.x, y / vec.y); }
    Vector2 operator + (float s) const { return Vector2(x + s, y + s); }
    Vector2 operator - (float s) const { return Vector2(x - s, y - s); }
    Vector2 operator * (float s) const { return Vector2(x * s, y * s); }
    Vector2 operator / (float s) const { return Vector2(x / s, y / s); }

    friend Vector2 operator + (float s, const Vector2 &vec) { return Vector2(s + vec.x, s + vec.y); }
    friend Vector2 operator - (float s, const Vector2 &vec) { return Vector2(s - vec.x, s - vec.y); }
    friend Vector2 operator * (float s, const Vector2 &vec) { return Vector2(s * vec.x, s * vec.y); }
    friend Vector2 operator / (float s, const Vector2 &vec) { return Vector2(s / vec.x, s / vec.y); }

    Vector2 &operator += (const Vector2 &vec) { return *this = *this + vec; }
    Vector2 &operator -= (const Vector2 &vec) { return *this = *this - vec; }
    Vector2 &operator *= (const Vector2 &vec) { return *this = *this * vec; }
    Vector2 &operator /= (const Vector2 &vec) { return *this = *this / vec; }
    Vector2 &operator += (float s) { return *this = *this + s; }
    Vector2 &operator -= (float s) { return *this = *this - s; }
    Vector2 &operator *= (float s) { return *this = *this * s; }
    Vector2 &operator /= (float s) { return *this = *this / s; }

    bool operator == (const Vector2 &vec) const { return x == vec.x && y == vec.y; }
    bool operator != (const Vector2 &vec) const { return x != vec.x || y != vec.y; }

    float lengthSquared() const { return x * x + y * y; }
    float length() const { return sqrtf(lengthSquared()); }
    float dot(const Vector2 &vec) const { return x * vec.x + y * vec.y; }
    Vector2 unit() const { return *this / length(); }
    void normalize() { *this = unit(); }
    static Vector2 lerp(const Vector2 &a, const Vector2 &b, float percent) { return a + (b - a) * percent; }

    float min() const { return ::min(x, y); }
    float max() const { return ::max(x, y); }
    static Vector2 min(const Vector2 &a, const Vector2 &b) { return Vector2(::min(a.x, b.x), ::min(a.y, b.y)); }
    static Vector2 max(const Vector2 &a, const Vector2 &b) { return Vector2(::max(a.x, b.x), ::max(a.y, b.y)); }

    float toAngle() const { return atan2f(y, x); }
    static Vector2 fromAngle(float theta) { return Vector2(cosf(theta), sinf(theta)); }
    static Vector2 uniform() { return fromAngle(frand() * M_2PI); }

    static const Vector2 X;
    static const Vector2 Y;
};

class Vector3
{
public:
    union
    {
        struct { float x, y, z; };
        float xyz[3];
    };

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    Vector3(const Vector3 &vec) : x(vec.x), y(vec.y), z(vec.z) {}

    Vector3 operator - () const { return Vector3(-x, -y, -z); }

    Vector3 operator + (const Vector3 &vec) const { return Vector3(x + vec.x, y + vec.y, z + vec.z); }
    Vector3 operator - (const Vector3 &vec) const { return Vector3(x - vec.x, y - vec.y, z - vec.z); }
    Vector3 operator * (const Vector3 &vec) const { return Vector3(x * vec.x, y * vec.y, z * vec.z); }
    Vector3 operator / (const Vector3 &vec) const { return Vector3(x / vec.x, y / vec.y, z / vec.z); }
    Vector3 operator + (float s) const { return Vector3(x + s, y + s, z + s); }
    Vector3 operator - (float s) const { return Vector3(x - s, y - s, z - s); }
    Vector3 operator * (float s) const { return Vector3(x * s, y * s, z * s); }
    Vector3 operator / (float s) const { return Vector3(x / s, y / s, z / s); }

    friend Vector3 operator + (float s, const Vector3 &vec) { return Vector3(s + vec.x, s + vec.y, s + vec.z); }
    friend Vector3 operator - (float s, const Vector3 &vec) { return Vector3(s - vec.x, s - vec.y, s - vec.z); }
    friend Vector3 operator * (float s, const Vector3 &vec) { return Vector3(s * vec.x, s * vec.y, s * vec.z); }
    friend Vector3 operator / (float s, const Vector3 &vec) { return Vector3(s / vec.x, s / vec.y, s / vec.z); }

    Vector3 &operator += (const Vector3 &vec) { return *this = *this + vec; }
    Vector3 &operator -= (const Vector3 &vec) { return *this = *this - vec; }
    Vector3 &operator *= (const Vector3 &vec) { return *this = *this * vec; }
    Vector3 &operator /= (const Vector3 &vec) { return *this = *this / vec; }
    Vector3 &operator += (float s) { return *this = *this + s; }
    Vector3 &operator -= (float s) { return *this = *this - s; }
    Vector3 &operator *= (float s) { return *this = *this * s; }
    Vector3 &operator /= (float s) { return *this = *this / s; }

    bool operator == (const Vector3 &vec) const { return x == vec.x && y == vec.y && z == vec.z; }
    bool operator != (const Vector3 &vec) const { return x != vec.x || y != vec.y || z != vec.z; }

    float lengthSquared() const { return x * x + y * y + z * z; }
    float length() const { return sqrtf(lengthSquared()); }
    float dot(const Vector3 &vec) const { return x * vec.x + y * vec.y + z * vec.z; }
    Vector3 cross(const Vector3 &vec) const { return Vector3(y * vec.z - z * vec.y, z * vec.x - x * vec.z, x * vec.y - y * vec.x); }
    Vector3 unit() const { return *this / length(); }
    void normalize() { *this = unit(); }
    static Vector3 lerp(const Vector3 &a, const Vector3 &b, float percent) { return a + (b - a) * percent; }

    float min() const { return ::min(x, ::min(y, z)); }
    float max() const { return ::max(x, ::max(y, z)); }
    static Vector3 min(const Vector3 &a, const Vector3 &b) { return Vector3(::min(a.x, b.x), ::min(a.y, b.y), ::min(a.z, b.z)); }
    static Vector3 max(const Vector3 &a, const Vector3 &b) { return Vector3(::max(a.x, b.x), ::max(a.y, b.y), ::max(a.z, b.z)); }

    Vector2 toAngles() const { return Vector2(atan2f(z, x), asinf(y / length())); }
    static Vector3 fromAngles(float theta, float phi) { return Vector3(cosf(theta) * cosf(phi), sinf(phi), sinf(theta) * cosf(phi)); }
    static Vector3 uniform() { return fromAngles(frand() * M_2PI, asinf(frand() * 2 - 1)); }

    static const Vector3 X;
    static const Vector3 Y;
    static const Vector3 Z;
};

inline std::ostream &operator << (std::ostream &out, const Vector2 &v) { return out << "<" << v.x << ", " << v.y << ">"; }
inline std::ostream &operator << (std::ostream &out, const Vector3 &v) { return out << "<" << v.x << ", " << v.y << ", " << v.z << ">"; }

#endif // VECTOR_H
