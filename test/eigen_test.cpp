#include <Eigen/Dense>
#include <iostream>

int main() {
  double dx = 1;
  double dy = 1;
  double dz = 1;
  Eigen::Matrix4f m;
  m << 1, 0, 0, 0,  //
      0, 1, 0, 0,   //
      0, 0, 1, 0,   //
      dx, dy, dz, 1;
  Eigen::Vector4f p0(1, 1, 0, 1);
  Eigen::Vector4f p1(2, 2, 0, 1);
  Eigen::Vector4f np0 = p0.transpose() * m;
  Eigen::Vector4f np1 = p1.transpose() * m;

  std::cout << np0 << std::endl;
  std::cout << np1 << std::endl;

  std::cout << np0[0] << std::endl;
  std::cout << np0[1] << std::endl;
}