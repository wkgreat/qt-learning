#pragma once

#include <Eigen/Dense>
#include <limits>
#include <random>

namespace qtgl {

using Vertice = Eigen::Vector4f;
using Vertices = Eigen::Matrix<float, Eigen::Dynamic, 4>;
using Index3 = Eigen::Vector3i;
using Indices3 = Eigen::Matrix<int, Eigen::Dynamic, 3>;
using Normal = Eigen::Vector3f;
using Normals = Eigen::Matrix<float, Eigen::Dynamic, 3>;
using NormIndex = Eigen::Vector3i;
using NormIndices = Eigen::Matrix<int, Eigen::Dynamic, 3>;

struct Position3 {
  float x;
  float y;
  float z;
};

struct Color {
  short R;
  short G;
  short B;
  short A;
  static Color random() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<short> distr(0, 255);
    return {distr(gen), distr(gen), distr(gen), distr(gen)};
  }
};

struct Color01 {
  float R;
  float G;
  float B;
  float A;
  static Color01 random() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distr(0, 1);
    return {distr(gen), distr(gen), distr(gen), distr(gen)};
  }
  void clamp() {
    R = R < 0 ? 0 : R;
    G = G < 0 ? 0 : G;
    B = B < 0 ? 0 : B;
    A = A < 0 ? 0 : A;
    R = R > 1 ? 1 : R;
    G = G > 1 ? 1 : G;
    B = B > 1 ? 1 : B;
    A = A > 1 ? 1 : A;
  }
  Color toColor() {
    Color c;
    c.R = static_cast<short>(R * 255);
    c.G = static_cast<short>(G * 255);
    c.B = static_cast<short>(B * 255);
    c.A = static_cast<short>(A * 255);
    return c;
  }
};

class Triangle {
 private:
  float f_alpha, f_beta, f_gamma;
  float _x0, _y0, _z0, _x1, _y1, _z1, _x2, _y2, _z2;
  float f01(float x, float y) { return (_y0 - _y1) * x + (_x1 - _x0) * y + _x0 * _y1 - _x1 * _y0; }
  float f12(float x, float y) { return (_y1 - _y2) * x + (_x2 - _x1) * y + _x1 * _y2 - _x2 * _y1; }
  float f20(float x, float y) { return (_y2 - _y0) * x + (_x0 - _x2) * y + _x2 * _y0 - _x0 * _y2; }

 public:
  Triangle(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2)
      : _x0(x0), _y0(y0), _z0(z0), _x1(x1), _y1(y1), _z1(z1), _x2(x2), _y2(y2), _z2(z2) {
    f_alpha = f12(x0, y0);
    f_beta = f20(x1, y1);
    f_gamma = f01(x2, y2);
  }
  Triangle(Vertice& p0, Vertice& p1, Vertice& p2) {
    this->_x0 = p0[0];
    this->_y0 = p0[1];
    this->_z0 = p0[2];
    this->_x1 = p1[0];
    this->_y1 = p1[1];
    this->_z1 = p1[2];
    this->_x2 = p2[0];
    this->_y2 = p2[1];
    this->_z2 = p2[2];
    f_alpha = f12(_x0, _y0);
    f_beta = f20(_x1, _y1);
    f_gamma = f01(_x2, _y2);
  }

  float x0() const { return _x0; }
  float y0() const { return _y0; }
  float z0() const { return _z0; }
  float x1() const { return _x1; }
  float y1() const { return _y1; }
  float z1() const { return _z1; }
  float x2() const { return _x2; }
  float y2() const { return _y2; }
  float z2() const { return _z2; }

  // 重心坐标
  struct BarycentricCoordnates {
    float alpha;
    float beta;
    float gamma;
  };

  // 求解重心坐标
  BarycentricCoordnates resovleBarycentricCoordnates(float x, float y) {
    BarycentricCoordnates coord;
    coord.alpha = f12(x, y) / f_alpha;
    coord.beta = f20(x, y) / f_beta;
    coord.gamma = f01(x, y) / f_gamma;
    return coord;
  }
};

struct Fragment {
  Color color;
  float depth;  // z-buffer
  constexpr static float DEPTH_INF = std::numeric_limits<float>::max() / 2;
  static Fragment init() { return {{255, 255, 255, 255}, DEPTH_INF}; }
};

using Fragments = std::vector<std::vector<Fragment>>;

}  // namespace qtgl