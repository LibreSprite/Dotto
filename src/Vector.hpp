#pragma once

#include <cmath>
#include <cstdint>
#include <iostream>

class UV {
public:
    union {
	float u, x;
    };
    union {
	float y, v;
    };

    float lengthSquared() const {
        return x*x + y*y;
    }

    float length() const {
        return sqrt(lengthSquared());
    }
};

using Vector2 = UV;

class Vector {
public:
    float x, y, z;
    Vector() : x{0.0f}, y{0.0f}, z{0.0f} {}
    Vector(float x, float y, float z) : x{x}, y{y}, z{z} {}
    Vector(const Vector& other) = default;
    Vector(Vector&& other) = default;

    Vector& set(float x, float y, float z) {
	this->x = x;
	this->y = y;
	this->z = z;
	return *this;
    }

    float lengthSquared() const {
        return x*x + y*y + z*z;
    }

    float length() const {
        return sqrt(lengthSquared());
    }

    Vector operator - () const {
        return {-x, -y, -z};
    }
};

using Vector3 = Vector;

class RGBA {
public:
    union {
	float r, x;
    };
    union {
	float g, y;
    };
    union {
	float b, z, h;
    };
    union {
	float a, w;
    };

    float lengthSquared() const {
        return x*x + y*y + z*z + w*w;
    }

    float length() const {
        return sqrt(lengthSquared());
    }
};

using Vector4 = RGBA;

class Rect {
public:
    int32_t x, y;
    uint32_t width, height;

    void clear() {
	x = y = 0;
	width = height = 0;
    }

    bool empty() {
	return !(width || height);
    }

    void expand(Rect o) {
	if (o.x < x) {
	    width += x - o.x;
	    x = o.x;
	}
	if (static_cast<int32_t>(o.x + o.width) > static_cast<int32_t>(x + width)) {
	    width = static_cast<int32_t>(o.x + o.width) - x;
	}
	if (o.y < y) {
	    height += y - o.y;
	    y = o.y;
	}
	if (static_cast<int32_t>(o.y + o.height) > static_cast<int32_t>(y + height)) {
	    height = static_cast<int32_t>(o.y + o.height) - y;
	}
    }
};

struct Color {
    union {
	struct {
	    uint8_t r;
	    uint8_t g;
	    uint8_t b;
	    uint8_t a;
	};
	uint32_t rgba;
    };
};

inline std::ostream& operator << (std::basic_ostream<char>& str, const RGBA& rgba) {
    str << "{R:" << rgba.r << ", G:" << rgba.g << ", B:" << rgba.b << ", A:" << rgba.a << "}";
    return str;
}

inline std::ostream& operator << (std::basic_ostream<char>& str, const Vector& v) {
    str << "{X:" << v.x << ", Y:" << v.y << ", Z:" << v.z << "}";
    return str;
}

inline std::ostream& operator << (std::basic_ostream<char>& str, const UV& uv) {
    str << "{U:" << uv.u << ", V:" << uv.v << "}";
    return str;
}
