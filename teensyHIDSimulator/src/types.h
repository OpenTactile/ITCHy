#ifndef TYPES_H
#define TYPES_H

#include <array>
#include <functional>
#include <cmath>


using vec2l = std::array<int64_t, 2>;
using vec2s = std::array<int16_t, 2>;

using vec2f = std::array<float, 2>;
using mat2f = std::array<float, 4>;

using color = std::array<unsigned char, 3>;
using byte = uint8_t;

inline vec2f mul(const mat2f& m, const vec2f& v)
{
    return {{
            m[0] * v[0] + m[1] * v[1],
            m[2] * v[0] + m[3] * v[1]
        }};
}

inline vec2f add(const vec2f& a, const vec2f& b)
{
    return {{
            a[0] + b[0],
            a[1] + b[1]
        }};
}

inline vec2f sub(const vec2f& a, const vec2f& b)
{
    return {{
            a[0] - b[0],
            a[1] - b[1]
        }};
}

inline vec2f mul(const vec2f& a, float b)
{
    return {{
            a[0] * b,
            a[1] * b
        }};
}

inline float len(const vec2f& v)
{
    return sqrt(v[0]*v[0] + v[1]*v[1]);
}

inline float len(const vec2l& v)
{
    return sqrt(v[0]*v[0] + v[1]*v[1]);
}


inline float invLen(const vec2f& v)
{
    return 1.0f/sqrt(v[0]*v[0] + v[1]*v[1]);
}

inline vec2f normalized(const vec2f& v)
{
    return mul(v, invLen(v));
}

inline float dot(const vec2f& a, const vec2f& b)
{
    return a[0]*b[0] + a[1]*b[1];
}

inline float cross(const vec2f& a, const vec2f& b)
{
    return a[0]*b[1] - a[1]*b[0];
}

struct SimulationParameters
{
    float mass;
    float stiffness;
    float damping;
    unsigned int updateRate;
};

struct SimulationState
{
    float time;
    float dt;

    float inverseMass;

    vec2f rawLeft = {{0.0f, 0.0f}};
    vec2f rawRight = {{0.0f, 0.0f}};

    vec2f positionLeft = {{0.0f, 0.0f}};
    vec2f velocityLeft = {{0.0f, 0.0f}};

    vec2f positionRight = {{0.0f, 0.0f}};
    vec2f velocityRight = {{0.0f, 0.0f}};

    vec2f position = {{0.0f, 0.0f}};
    vec2f velocity = {{0.0f, 0.0f}};
    float angle = 0.0f;
    float angularVelocity = 0.0f;
    float torque = 0.0;

    mat2f rotation = {{0.0f, 0.0f, 0.0f, 0.0f}};
};

#endif // TYPES_H
