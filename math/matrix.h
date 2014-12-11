#pragma once
#include "vector.h"

#include <cmath>
#include <cassert>
#include <cstdlib>
#include <initializer_list>

template<size_t N>
struct Matrix {
    float elements[N][N];
    
	Matrix() {
		for(size_t i = 0; i < N; ++i) {
			for(size_t j = 0; j < N; ++j) {
				elements[i][j] = 0.0f;
			}
		}
	}
    Matrix(std::initializer_list<std::initializer_list<float>> rows) {
        size_t i = 0, j = 0;
        assert(rows.size() == N);
        for(auto &row : rows) {
            j = 0;
            assert(row.size() == N);
            for(auto element : row) {
                elements[i][j++] = element;
            }
            ++i;
        }
    }
    
    Matrix operator*(const Matrix &other) const{
        Matrix r;
        for(unsigned i = 0; i < N; ++i) {
            for(unsigned j = 0; j < N; ++j) {
                for(unsigned k = 0; k < N; ++k) {
                    r.elements[i][j] += this->elements[i][k] * other.elements[k][j];
                }
            }
        }
        return r;
    }
};

typedef Matrix<4> Matrix4;

Vector4 operator*(const Matrix4 &matrix, const Vector4 &v) {
    auto m = matrix.elements;
    return {
        m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z + m[0][3]*v.w,
        m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z + m[1][3]*v.w,
        m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z + m[2][3]*v.w,
        m[3][0]*v.x + m[3][1]*v.y + m[3][2]*v.z + m[3][3]*v.w
    };
}

inline Matrix4 make_identity_matrix() {
    return {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    };
}


inline Matrix4 make_translate_matrix(Vector4 t) {
    return {
        {1, 0, 0, t.x},
        {0, 1, 0, t.y},
        {0, 0, 1, t.z},
        {0, 0, 0, 1  }
    };
}

inline Matrix4 make_scale_matrix(Vector4 s) {
	return{
			{ s.x, 0,   0,   0   },
			{ 0,   s.y, 0,   0   },
			{ 0,   0,   s.z, 0   },
			{ 0,   0,   0,   s.w }
	};
}


inline Matrix4 make_perspective_matrix(float S, float zNear, float zFar) {
    const float N = zNear, F = zFar;
    return {
        {S, 0,  0,           0          },
        {0, S,  0,           0          },
        {0, 0,  (F+N)/(N-F), 2*N*F/(N-F)},
        {0, 0,  -1,          0          }
    };
}

inline Matrix4 make_lookat_matrix(const Vector4 &eye, const Vector4 &center) {
    Vector4 j {0, 1, 0};
    const Vector4 k = (center - eye).normalized();
    const Vector4 i = k.cross(j).normalized();
    j = i.cross(k);
    return {
        { i.x,  i.y,  i.z, -i.dot(eye)},
        { j.x,  j.y,  j.z, -j.dot(eye)},
        {-k.x, -k.y, -k.z,  k.dot(eye)},
        { 0,    0,    0,    1         }
    };
}
