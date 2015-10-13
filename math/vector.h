#pragma once
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <initializer_list>

struct Vector4 {
  float x = 0, y = 0, z = 0, w = 0;

  inline Vector4(float x_ = 0.f, float y_ = 0.f, float z_ = 0.f, float w_ = 1.f)
      : x(x_), y(y_), z(z_), w(w_) {}

  float dot(const Vector4 &other) const {
    return this->x * other.x + this->y * other.y + this->z * other.z;
  }
  Vector4 cross(const Vector4 &other) const {
    return {this->y * other.z - this->z * other.y,
            this->z * other.x - this->x * other.z,
            this->x * other.y - this->y * other.x, 0};
  }
  Vector4 operator+(const Vector4 &other) const {
    return {this->x + other.x, this->y + other.y, this->z + other.z,
            this->w + other.w};
  }
  Vector4 operator-(const Vector4 &other) const {
    return {this->x - other.x, this->y - other.y, this->z - other.z,
            this->w - other.w};
  }
  Vector4 operator*(float a) const { return {a * x, a * y, a * z, a * w}; }
  Vector4 operator/(float a) const { return {x / a, y / a, z / a, w / a}; }
  float lensq() const { return x * x + y * y + z * z; }
  float len() const { return sqrt(lensq()); }
  Vector4 normalized() const { return (*this) / len(); }
};
