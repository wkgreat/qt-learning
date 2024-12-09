#pragma once

#include <Eigen/Dense>

namespace qtgl {

using Vertice = Eigen::Vector4f;
using Vertices = Eigen::Matrix<float, Eigen::Dynamic, 4>;
using Index3 = Eigen::Vector3i;
using Indices3 = Eigen::Matrix<int, Eigen::Dynamic, 3>;

}  // namespace qtgl