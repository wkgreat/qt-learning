#pragma once

#include "define.hpp"

namespace qtgl {

struct AffineUtils {
  static Vertices affine(Vertices& vtx, Eigen::Matrix4d& mtx) { return vtx * mtx; }
  static Vertices translate(Vertices& vtx, double x, double y, double z) {
    Eigen::Matrix4d m;
    m << 1, 0, 0, 0,  //
        0, 1, 0, 0,   //
        0, 0, 1, 0,   //
        x, y, z, 1;
    return affine(vtx, m);
  };
  static Vertices rotate_x(Vertices& vtx, double a) {
    Eigen::Matrix4d m;
    m << 1, 0, 0, 0,            //
        0, cos(a), sin(a), 0,   //
        0, -sin(a), cos(a), 0,  //
        0, 0, 0, 1;
    return affine(vtx, m);
  };
  static Vertices rotate_y(Vertices& vtx, double a) {
    Eigen::Matrix4d m;
    m << cos(a), 0, -sin(a), 0,  //
        0, 1, 0, 0,              //
        sin(a), 0, cos(a), 0,    //
        0, 0, 0, 1;
    return affine(vtx, m);
  };
  static Vertices rotate_z(Vertices& vtx, double a) {
    Eigen::Matrix4d m;
    m << cos(a), sin(a), 0, 0,  //
        -sin(a), cos(a), 0, 0,  //
        0, 0, 1, 0,             //
        0, 0, 0, 1;
    return affine(vtx, m);
  };
  static Vertices scale(Vertices& vtx, double x, double y, double z) {
    Eigen::Matrix4d m;
    m << x, 0, 0, 0,  //
        0, y, 0, 0,   //
        0, 0, z, 0,   //
        0, 0, 0, 1;
    return affine(vtx, m);
  };
};

}  // namespace qtgl