#pragma once

#include "define.hpp"

namespace qtgl {

struct AffineUtils {
  static Vertices affine(Vertices& vtx, Eigen::Matrix4d& mtx) { return vtx * mtx; }
  static Normals norm_affine(Normals& vtx, Eigen::Matrix3d& mtx) {
    return vtx * (mtx.inverse().transpose());
  }
  static Vertices translate(Vertices& vtx, double x, double y, double z) {
    Eigen::Matrix4d m;
    m << 1, 0, 0, 0,  //
        0, 1, 0, 0,   //
        0, 0, 1, 0,   //
        x, y, z, 1;
    return affine(vtx, m);
  };
  static Normals normal_translate(Normals& norms, double x, double y, double z) { return norms; };
  static Vertices rotate_x(Vertices& vtx, double a) {
    Eigen::Matrix4d m;
    m << 1, 0, 0, 0,            //
        0, cos(a), sin(a), 0,   //
        0, -sin(a), cos(a), 0,  //
        0, 0, 0, 1;
    return affine(vtx, m);
  };
  static Normals normal_rotate_x(Normals& norms, double a) {
    Eigen::Matrix3d m;
    m << 1, 0, 0,            //
        0, cos(a), sin(a),   //
        0, -sin(a), cos(a);  //
    return norm_affine(norms, m);
  }
  static Vertices rotate_y(Vertices& vtx, double a) {
    Eigen::Matrix4d m;
    m << cos(a), 0, -sin(a), 0,  //
        0, 1, 0, 0,              //
        sin(a), 0, cos(a), 0,    //
        0, 0, 0, 1;
    return affine(vtx, m);
  };
  static Normals normal_rotate_y(Normals& norms, double a) {
    Eigen::Matrix3d m;
    m << cos(a), 0, -sin(a),  //
        0, 1, 0,              //
        sin(a), 0, cos(a);    //
    return norm_affine(norms, m);
  };
  static Vertices rotate_z(Vertices& vtx, double a) {
    Eigen::Matrix4d m;
    m << cos(a), sin(a), 0, 0,  //
        -sin(a), cos(a), 0, 0,  //
        0, 0, 1, 0,             //
        0, 0, 0, 1;
    return affine(vtx, m);
  };
  static Normals normal_rotate_z(Normals& norms, double a) {
    Eigen::Matrix3d m;
    m << cos(a), sin(a), 0,  //
        -sin(a), cos(a), 0,  //
        0, 0, 1;
    return norm_affine(norms, m);
  };
  static Vertices scale(Vertices& vtx, double x, double y, double z) {
    Eigen::Matrix4d m;
    m << x, 0, 0, 0,  //
        0, y, 0, 0,   //
        0, 0, z, 0,   //
        0, 0, 0, 1;
    return affine(vtx, m);
  };
  static Normals norm_scale(Normals& norms, double x, double y, double z) { return norms; };
};

}  // namespace qtgl