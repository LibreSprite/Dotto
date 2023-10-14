#pragma once

#include "Vector.hpp"

class Matrix {
public:
    float v[16];

    Matrix() {
        identity();
    }

    Matrix(const Matrix& other) = default;

    Matrix& identity() {
        v[ 0] = 1; v[ 1] = 0; v[ 2] = 0; v[ 3] = 0;
        v[ 4] = 0; v[ 5] = 1; v[ 6] = 0; v[ 7] = 0;
        v[ 8] = 0; v[ 9] = 0; v[10] = 1; v[11] = 0;
        v[12] = 0; v[13] = 0; v[14] = 0; v[15] = 1;
        return *this;
    }

    Matrix& operator *= (const Matrix& other) {
        auto r = other.v;

        float v0 = v[0], v1 = v[1], v2 = v[2], v3 = v[3];
        v[0] = r[ 0]*v0 + r[ 4]*v1 + r[ 8]*v2 + r[ 12]*v3;
        v[1] = r[ 1]*v0 + r[ 5]*v1 + r[ 9]*v2 + r[ 13]*v3;
        v[2] = r[ 2]*v0 + r[ 6]*v1 + r[10]*v2 + r[ 14]*v3;
        v[3] = r[ 3]*v0 + r[ 7]*v1 + r[11]*v2 + r[ 15]*v3;

        v0 = v[4]; v1 = v[5]; v2 = v[6]; v3 = v[7];
        v[4] = r[ 0]*v0 + r[ 4]*v1 + r[ 8]*v2 + r[ 12]*v3;
        v[5] = r[ 1]*v0 + r[ 5]*v1 + r[ 9]*v2 + r[ 13]*v3;
        v[6] = r[ 2]*v0 + r[ 6]*v1 + r[10]*v2 + r[ 14]*v3;
        v[7] = r[ 3]*v0 + r[ 7]*v1 + r[11]*v2 + r[ 15]*v3;

        v0 = v[8]; v1 = v[9]; v2 = v[10]; v3 = v[11];
        v[ 8] = r[ 0]*v0 + r[ 4]*v1 + r[ 8]*v2 + r[ 12]*v3;
        v[ 9] = r[ 1]*v0 + r[ 5]*v1 + r[ 9]*v2 + r[ 13]*v3;
        v[10] = r[ 2]*v0 + r[ 6]*v1 + r[10]*v2 + r[ 14]*v3;
        v[11] = r[ 3]*v0 + r[ 7]*v1 + r[11]*v2 + r[ 15]*v3;

        v0 = v[12]; v1 = v[13]; v2 = v[14]; v3 = v[15];
        v[12] = r[ 0]*v0 + r[ 4]*v1 + r[ 8]*v2 + r[ 12]*v3;
        v[13] = r[ 1]*v0 + r[ 5]*v1 + r[ 9]*v2 + r[ 13]*v3;
        v[14] = r[ 2]*v0 + r[ 6]*v1 + r[10]*v2 + r[ 14]*v3;
        v[15] = r[ 3]*v0 + r[ 7]*v1 + r[11]*v2 + r[ 15]*v3;

        return *this;
    }

    Matrix& setPosition(const Vector& pos) {
        setPosition(pos.x, pos.y, pos.z);
        return *this;
    }

    Matrix& setPosition(float x, float y, float z) {
        v[ 3] = x;
        v[ 7] = y;
        v[11] = z;
        return *this;
    }

    Matrix& set(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33) {

	v[0] = m00;	 v[1] = m01;	 v[2] = m02;	 v[3] = m03;
	v[4] = m10;	 v[5] = m11;	 v[6] = m12;	 v[7] = m13;
	v[8] = m20;	 v[9] = m21;	 v[10] = m22;	 v[11] = m23;
        v[12] = m30;	 v[13] = m31;	 v[14] = m32;	 v[15] = m33;

	return *this;
    }

    Matrix& setCM(float m00, float m01, float m02, float m03,
		  float m10, float m11, float m12, float m13,
		  float m20, float m21, float m22, float m23,
		  float m30, float m31, float m32, float m33) {

	v[0] = m00;	 v[4] = m01;	 v[8] = m02;	 v[12] = m03;
	v[1] = m10;	 v[5] = m11;	 v[9] = m12;	 v[13] = m13;
	v[2] = m20;	 v[6] = m21;	 v[10] = m22;	 v[14] = m23;
        v[3] = m30;	 v[7] = m31;	 v[11] = m32;	 v[15] = m33;

	return *this;
    }

    static Matrix rotation(float s, Vector3 axis) {
	// s *= 0.5f;

	// Vector3 v = axis;
	// v.normalize();                  // convert to unit vector
	float sine = sinf(s);
	float cos = cosf(s);
	float x = axis.x * sine;
	float y = axis.y * sine;
	float z = axis.z * sine;

	// compute common values
	float x2  = x + x;
	float y2  = y + y;
	float z2  = z + z;
	float xx2 = x * x2;
	float xy2 = x * y2;
	float xz2 = x * z2;
	float yy2 = y * y2;
	float yz2 = y * z2;
	float zz2 = z * z2;
	float sx2 = cos * x2;
	float sy2 = cos * y2;
	float sz2 = cos * z2;

	Matrix mat;

	// build 4x4 matrix (column-major) and return
	mat.setCM(1 - (yy2 + zz2),  xy2 + sz2,        xz2 - sy2,        0, // column 0
		  xy2 - sz2,        1 - (xx2 + zz2),  yz2 + sx2,        0, // column 1
		  xz2 + sy2,        yz2 - sx2,        1 - (xx2 + yy2),  0, // column 2
		  0,                0,                0,                1);// column 3

	// for non-unit quaternion
	// ss+xx-yy-zz, 2xy+2sz,     2xz-2sy,     0
	// 2xy-2sz,     ss-xx+yy-zz, 2yz-2sx,     0
	// 2xz+2sy,     2yz+2sx,     ss-xx-yy+zz, 0
	// 0,           0,           0,           1

	return mat;
    }

    static Matrix position(float x, float y, float z) {
        Matrix m;
        m.setPosition(x, y, z);
        return m;
    }

    static Matrix projection(float width, float height, float near, float far, float FOV) {
        const float ar = width / height;
        const float range = near - far;
        const float tanHalfFOV = tanf(FOV / 2.0);
        Matrix m;
        m.v[ 0] = 1.0f / (tanHalfFOV * ar);
        m.v[ 1] = 0.0f;
        m.v[ 2] = 0.0f;
        m.v[ 3] = 0.0f;

        m.v[ 4] = 0.0f;
        m.v[ 5] = 1.0f / tanHalfFOV;
        m.v[ 6] = 0.0f;
        m.v[ 7] = 0.0f;

        m.v[ 8] = 0.0f;
        m.v[ 9] = 0.0f;
        m.v[10] = (-near - far) / range;
        m.v[11] = 2.0f * far * near / range;

        m.v[12] = 0.0f;
        m.v[13] = 0.0f;
        m.v[14] = 1.0f;
        m.v[15] = 0.0f;
        return m;
    }
};
