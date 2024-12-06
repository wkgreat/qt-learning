#pragma once

#include "define.hpp"

namespace qtgl {

struct AffineUtils {
  static Vertices affine(Vertices& vtx, Eigen::Matrix4f& mtx) { return vtx * mtx; }
  static Vertices translate(Vertices& vtx, float x, float y, float z) {
    Eigen::Matrix4f m;
    m << 1, 0, 0, 0,  //
        0, 1, 0, 0,   //
        0, 0, 1, 0,   //
        x, y, z, 1;
    return affine(vtx, m);
  };
  static Vertices rotate_x(Vertices& vtx, float a) {
    Eigen::Matrix4f m;
    m << 1, 0, 0, 0,            //
        0, cos(a), sin(a), 0,   //
        0, -sin(a), cos(a), 0,  //
        0, 0, 0, 1;
    return affine(vtx, m);
  };
  static Vertices rotate_y(Vertices& vtx, float a) {
    Eigen::Matrix4f m;
    m << cos(a), 0, -sin(a), 0,  //
        0, 1, 0, 0,              //
        sin(a), 0, cos(a), 0,    //
        0, 0, 0, 1;
    return affine(vtx, m);
  };
  static Vertices rotate_z(Vertices& vtx, float a) {
    Eigen::Matrix4f m;
    m << cos(a), sin(a), 0, 0,  //
        -sin(a), cos(a), 0, 0,  //
        0, 0, 1, 0,             //
        0, 0, 0, 1;
    return affine(vtx, m);
  };
  static Vertices scale(Vertices& vtx, float x, float y, float z) {
    Eigen::Matrix4f m;
    m << x, 0, 0, 0,  //
        0, y, 0, 0,   //
        0, 0, z, 0,   //
        0, 0, 0, 1;
    return affine(vtx, m);
  };
};

}  // namespace qtgl