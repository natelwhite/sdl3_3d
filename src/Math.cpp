#include "Math.hpp"

Vector3 Vector3::normalize() const {
	float mag { 
		SDL_sqrtf(
			(this->at(0) * this->at(0)) +
			(this->at(1) * this->at(1)) +
			(this->at(2) * this->at(2))
		)
	};
	return Vector3 {
		this->at(0) / mag,
		this->at(1) / mag,
		this->at(2) / mag
	};
}
float Vector3::dot(const Vector3 &other) const {
	return (this->at(0) * other.at(0)) + (this->at(1) * other.at(1)) + (this->at(2) * other.at(2));
}
Vector3 Vector3::cross(const Vector3 &other) const {
	return {
		this->at(1) * other.at(2) - other.at(1) * this->at(2),
		-(this->at(0) * other.at(2) - other.at(0) * this->at(2)),
		this->at(0) * other.at(1) - other.at(0) * this->at(1)
	};
}
Matrix4x4 Matrix4x4:: operator * (const Matrix4x4 &other) {
	auto mul = [this, other](const int &row, const int &col) -> float {
		return 
			this->at(row).at(0) * other.at(0).at(col) +
			this->at(row).at(1) * other.at(1).at(col) +
			this->at(row).at(2) * other.at(2).at(col) +
			this->at(row).at(3) * other.at(3).at(col);
	};
	return Matrix4x4 {
		Vector4{ mul(0, 0), mul(0, 1), mul(0, 2), mul(0, 3) },
		Vector4{ mul(1, 0), mul(1, 1), mul(1, 2), mul(1, 3) },
		Vector4{ mul(2, 0), mul(2, 1), mul(2, 2), mul(2, 3) },
		Vector4{ mul(3, 0), mul(3, 1), mul(3, 2), mul(3, 3) }
	};
}
