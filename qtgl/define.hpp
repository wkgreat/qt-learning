#pragma once

#include <Eigen/Dense>
#include <limits>
#include <random>

namespace qtgl {

using Vertice = Eigen::Vector4d;
using Vertices = Eigen::Matrix<double, Eigen::Dynamic, 4>;
using Index3 = Eigen::Vector3i;
using Indices3 = Eigen::Matrix<int, Eigen::Dynamic, 3>;
using Normal = Eigen::Vector3d;
using Normals = Eigen::Matrix<double, Eigen::Dynamic, 3>;
using NormIndex = Eigen::Vector3i;
using NormIndices = Eigen::Matrix<int, Eigen::Dynamic, 3>;
using TexCoord = Eigen::Vector2d;
using TexCoords = Eigen::Matrix<double, Eigen::Dynamic, 2>;

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
  double R;
  double G;
  double B;
  double A;
  Color01 operator+(Color01& c) {
    Color01 r;
    r.R = R + c.R;
    r.G = G + c.G;
    r.B = B + c.B;
    r.A = A + c.A;
    return r;
  }
  static Color01 random() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> distr(0, 1);
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
  double f_alpha, f_beta, f_gamma;
  double _x0, _y0, _z0, _x1, _y1, _z1, _x2, _y2, _z2;
  double f01(double x, double y) {
    return (_y0 - _y1) * x + (_x1 - _x0) * y + _x0 * _y1 - _x1 * _y0;
  }
  double f12(double x, double y) {
    return (_y1 - _y2) * x + (_x2 - _x1) * y + _x1 * _y2 - _x2 * _y1;
  }
  double f20(double x, double y) {
    return (_y2 - _y0) * x + (_x0 - _x2) * y + _x2 * _y0 - _x0 * _y2;
  }

 public:
  Triangle(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2,
           double z2)
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

  double x0() const { return _x0; }
  double y0() const { return _y0; }
  double z0() const { return _z0; }
  double x1() const { return _x1; }
  double y1() const { return _y1; }
  double z1() const { return _z1; }
  double x2() const { return _x2; }
  double y2() const { return _y2; }
  double z2() const { return _z2; }

  // 重心坐标
  struct BarycentricCoordnates {
    double alpha;
    double beta;
    double gamma;
  };

  // 求解重心坐标
  BarycentricCoordnates resovleBarycentricCoordnates(double x, double y) {
    BarycentricCoordnates coord;
    coord.alpha = f12(x, y) / f_alpha;
    coord.beta = f20(x, y) / f_beta;
    coord.gamma = f01(x, y) / f_gamma;
    return coord;
  }
};

class Texture;

class Triangle2 {
 private:
  Vertice p0, p1, p2;
  Vertice hp0, hp1, hp2;
  Normal n0, n1, n2;
  TexCoord t0, t1, t2;
  bool hastexture = false;
  double f_alpha, f_beta, f_gamma;
  double f01(double x, double y) {
    return (hy0() - hy1()) * x + (hx1() - hx0()) * y + hx0() * hy1() - hx1() * hy0();
  }
  double f12(double x, double y) {
    return (hy1() - hy2()) * x + (hx2() - hx1()) * y + hx1() * hy2() - hx2() * hy1();
  }
  double f20(double x, double y) {
    return (hy2() - hy0()) * x + (hx0() - hx2()) * y + hx2() * hy0() - hx0() * hy2();
  }

 public:
  Triangle2(Vertice& p0, Vertice& p1, Vertice& p2, Normal& n0, Normal& n1, Normal& n2) {
    this->p0 = p0;
    this->p1 = p1;
    this->p2 = p2;

    this->n0 = n0;
    this->n1 = n1;
    this->n2 = n2;

    this->hp0 = p0 / p0[3];
    this->hp1 = p1 / p1[3];
    this->hp2 = p2 / p2[3];

    f_alpha = f12(hx0(), hy0());
    f_beta = f20(hx1(), hy1());
    f_gamma = f01(hx2(), hy2());

    hastexture = false;
  }
  Triangle2(Vertice& p0, Vertice& p1, Vertice& p2, Normal& n0, Normal& n1, Normal& n2, TexCoord& t0,
            TexCoord& t1, TexCoord& t2) {
    this->p0 = p0;
    this->p1 = p1;
    this->p2 = p2;

    this->n0 = n0;
    this->n1 = n1;
    this->n2 = n2;

    this->t0 = t0;
    this->t1 = t1;
    this->t2 = t2;

    this->hp0 = p0 / p0[3];
    this->hp1 = p1 / p1[3];
    this->hp2 = p2 / p2[3];

    f_alpha = f12(hx0(), hy0());
    f_beta = f20(hx1(), hy1());
    f_gamma = f01(hx2(), hy2());

    hastexture = true;
  }

  inline double hx0() const { return hp0[0]; }
  inline double hy0() const { return hp0[1]; }
  inline double hz0() const { return hp0[2]; }

  inline double hx1() const { return hp1[0]; }
  inline double hy1() const { return hp1[1]; }
  inline double hz1() const { return hp1[2]; }

  inline double hx2() const { return hp2[0]; }
  inline double hy2() const { return hp2[1]; }
  inline double hz2() const { return hp2[2]; }

  Normal& getNormal0() { return n0; }
  Normal& getNormal1() { return n1; }
  Normal& getNormal2() { return n2; }

  bool getHasTexture() const { return hastexture; }

  TexCoord& getTexCoord0() { return t0; }
  TexCoord& getTexCoord1() { return t1; }
  TexCoord& getTexCoord2() { return t2; }

  double w0() const { return p0[3]; }
  double w1() const { return p1[3]; }
  double w2() const { return p2[3]; }

  // 重心坐标
  struct BarycentricCoordnates {
    double alpha;
    double beta;
    double gamma;
  };

  // 求解重心坐标
  BarycentricCoordnates resovleBarycentricCoordnates(double x, double y) {
    BarycentricCoordnates coord;
    coord.alpha = f12(x, y) / f_alpha;
    coord.beta = f20(x, y) / f_beta;
    coord.gamma = f01(x, y) / f_gamma;
    return coord;
  }
};

struct Fragment {
  Color01 color;
  double depth;  // z-buffer
  constexpr static double DEPTH_INF = std::numeric_limits<double>::max() / 2;
  static Fragment init() { return {{255, 255, 255, 255}, DEPTH_INF}; }
};

using Fragments = std::vector<std::vector<Fragment>>;

}  // namespace qtgl