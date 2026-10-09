#ifndef MATH3D_H
#define MATH3D_H

#include <cmath>

struct Vector3 {
    float x, y, z;

    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
    Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
    Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }

    float dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }

    Vector3 cross(const Vector3& v) const {
        return Vector3(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }

    float length() const { return std::sqrt(x * x + y * y + z * z); }

    Vector3 normalized() const {
        float len = length();
        if (len < 0.00001f) return Vector3(0, 0, 0);
        return Vector3(x / len, y / len, z / len);
    }
};

struct Matrix4 {
    float m[16]; // column-major or row-major (row-major preferred for simplicity)

    Matrix4() {
        for (int i = 0; i < 16; ++i) m[i] = 0.0f;
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }

    static Matrix4 identity() {
        return Matrix4();
    }

    static Matrix4 translation(float tx, float ty, float tz) {
        Matrix4 res;
        res.m[3] = tx;
        res.m[7] = ty;
        res.m[11] = tz;
        return res;
    }

    static Matrix4 scale(float sx, float sy, float sz) {
        Matrix4 res;
        res.m[0] = sx;
        res.m[5] = sy;
        res.m[10] = sz;
        return res;
    }

    static Matrix4 rotationY(float angleRad) {
        Matrix4 res;
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        res.m[0] = c;  res.m[2] = s;
        res.m[8] = -s; res.m[10] = c;
        return res;
    }

    static Matrix4 rotationX(float angleRad) {
        Matrix4 res;
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        res.m[5] = c;  res.m[6] = -s;
        res.m[9] = s;  res.m[10] = c;
        return res;
    }

    static Matrix4 perspective(float fovRad, float aspect, float nearZ, float farZ) {
        Matrix4 res;
        float tanHalfFov = std::tan(fovRad / 2.0f);
        for(int i=0; i<16; ++i) res.m[i] = 0.0f;
        res.m[0] = 1.0f / (aspect * tanHalfFov);
        res.m[5] = 1.0f / tanHalfFov;
        res.m[10] = -(farZ + nearZ) / (farZ - nearZ);
        res.m[11] = -(2.0f * farZ * nearZ) / (farZ - nearZ);
        res.m[14] = -1.0f;
        return res;
    }

    Vector3 transformPoint(const Vector3& p) const {
        float x = m[0]*p.x + m[1]*p.y + m[2]*p.z + m[3];
        float y = m[4]*p.x + m[5]*p.y + m[6]*p.z + m[7];
        float z = m[8]*p.x + m[9]*p.y + m[10]*p.z + m[11];
        float w = m[12]*p.x + m[13]*p.y + m[14]*p.z + m[15];
        if (std::abs(w) > 0.00001f) {
            return Vector3(x / w, y / w, z / w);
        }
        return Vector3(x, y, z);
    }
};

#endif // MATH3D_H
