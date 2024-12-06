#pragma once

#include "define.hpp"
#include "mathutils.hpp"

namespace qtgl {
enum class GLProjectionMode {
  ORTHOGRAPHIC,  // 正射投影
  PRESPECTIVE    // 透视投影
};

class GLProjection {
 public:
  float height, width;
  float near, far;
  GLProjectionMode mode;
  GLProjection()
      : height(768), width(1024), near(0.1f), far(100), mode(GLProjectionMode::PRESPECTIVE) {}
  GLProjection(float height, float width, float near, float far, GLProjectionMode mode)
      : height(height), width(width), near(near), far(far), mode(mode) {}
  Eigen::Matrix4f orthographicProjMatrix() {
    float hfov = MathUtils::PI / 3;
    float vfov = hfov * (height / width);
    float right = tan(hfov / 2) * near;
    float left = -right;
    float top = tan(vfov / 2) * near;
    float bottom = -top;

    float m00 = 2 / (right - left);
    float m11 = 2 / (top - bottom);
    float m22 = 2 / (near - far);
    float m30 = -(right + left) / (right - left);
    float m31 = -(top + bottom) / (top - bottom);
    float m32 = -(near + far) / (near - far);

    Eigen::Matrix4f projMtx;
    projMtx << m00, 0, 0, 0,  //
        0, m11, 0, 0,         //
        0, 0, m22, 0,         //
        m30, m31, m32, 1;
    return projMtx;
  }
  Eigen::Matrix4f perspectiveProjMatrix() {
    float hfov = MathUtils::PI / 3;
    float vfov = hfov * (height / width);
    float right = tan(hfov / 2) * near;
    float left = -right;
    float top = tan(vfov / 2) * near;
    float bottom = -top;
    float m00 = 2 / (right - left);
    float m11 = 2 / (top - bottom);
    float m22 = (far + near) / (far - near);
    float m32 = -2 * near * far / (far - near);
    Eigen::Matrix4f projMtx;
    projMtx << m00, 0, 0, 0,  //
        0, m11, 0, 0,         //
        0, 0, m22, 1,         //
        0, 0, m32, 0;
    return projMtx;
  }
  Eigen::Matrix4f projMatrix() {
    if (mode == GLProjectionMode::ORTHOGRAPHIC) {
      return orthographicProjMatrix();
    } else {
      return perspectiveProjMatrix();
    }
  }
};
}  // namespace qtgl