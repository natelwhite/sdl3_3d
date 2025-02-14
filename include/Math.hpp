#pragma once
#include <array>
#include <SDL3/SDL_stdinc.h>

struct Vector3 : std::array<float, 3> {
	Vector3 normalize() const;
	float dot(const Vector3 &other) const;
	Vector3 cross(const Vector3 &other) const;
};
struct Vector4 : std::array<float, 4> {};
struct Matrix4x4 : std::array<Vector4, 4> {
	Matrix4x4 operator * (const Matrix4x4 &other);
};
