#include "Math.hpp"

Vector3 Vector3::normalize() const {
	float mag { SDL_sqrtf( SDL_exp(at(0)) + SDL_exp(at(1)) + SDL_exp(at(2)) ) };
	return Vector3 { at(0) / mag, at(1) / mag, at(2) / mag };
}
float Vector3::dot(const Vector3 &other) const {
	return (at(0) * other.at(0)) + (at(1) * other.at(1)) + (at(2) * other.at(2));
}
Vector3 Vector3::cross(const Vector3 &other) const {
	return {
		at(1) * other.at(2) - other.at(1) * at(2),
		-(at(0) * other.at(2) - other.at(0) * at(2)),
		at(0) * other.at(1) - other.at(0) * at(1)
	};
}
Matrix4x4 Matrix4x4:: operator * (const Matrix4x4 &other) {
	auto mul = [this, other](const int &r, const int &c) -> float {
		return 
			at(r).at(0) * other.at(0).at(c) +
			at(r).at(1) * other.at(1).at(c) +
			at(r).at(2) * other.at(2).at(c) +
			at(r).at(3) * other.at(3).at(c);
	};
	return Matrix4x4 {
		Vector4{ mul(0, 0), mul(0, 1), mul(0, 2), mul(0, 3) },
		Vector4{ mul(1, 0), mul(1, 1), mul(1, 2), mul(1, 3) },
		Vector4{ mul(2, 0), mul(2, 1), mul(2, 2), mul(2, 3) },
		Vector4{ mul(3, 0), mul(3, 1), mul(3, 2), mul(3, 3) }
	};
}
