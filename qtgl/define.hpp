#pragma once

#include <Eigen/Dense>
#include <random>

namespace qtgl {

using Vertice = Eigen::Vector4f;
using Vertices = Eigen::Matrix<float, Eigen::Dynamic, 4>;
using Index3 = Eigen::Vector3i;
using Indices3 = Eigen::Matrix<int, Eigen::Dynamic, 3>;

struct Color {
  short R;
  short G;
  short B;
  short A;
  static Color random() {
    std::random_device rd;
    std::mt19937 gen(rd());
    Color c;
    std::uniform_int_distribution<> distr(0, 255);
    c.R = distr(gen);
    c.G = distr(gen);
    c.B = distr(gen);
    c.A = distr(gen);
    return c;
  }
};

class Triangle {
 private:
  float f_alpha, f_beta, f_gamma;
  float f01(float x, float y) { return (y0 - y1) * x + (x1 - x0) * y + x0 * y1 - x1 * y0; }
  float f12(float x, float y) { return (y1 - y2) * x + (x2 - x1) * y + x1 * y2 - x2 * y1; }
  float f20(float x, float y) { return (y2 - y0) * x + (x0 - x2) * y + x2 * y0 - x0 * y2; }

 public:
  float x0, y0, x1, y1, x2, y2;
  Triangle(float x0, float y0, float x1, float y1, float x2, float y2)
      : x0(x0), y0(y0), x1(x1), y1(y1), x2(x2), y2(y2) {
    f_alpha = f12(x0, y0);
    f_beta = f20(x1, y1);
    f_gamma = f01(x2, y2);
  }

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

}  // namespace qtgl