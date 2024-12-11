#pragma once

#include "define.hpp"

namespace qtgl {

struct GLLight {
  Color01 intensity;
  virtual Eigen::Vector3f uvLight(Position3& pos) = 0;  // l unit vector
};

struct DirectionalGLLight : public GLLight {
  Eigen::Vector3f d;  // direction
  Eigen::Vector3f uvLight(Position3& pos) { return (d * -1).normalized(); }
};

struct PointGLLight : public GLLight {
  Position3 position;
  Eigen::Vector3f uvLight(Position3& pos) {
    Eigen::Vector3f v{position.x - pos.x, position.y - pos.y, position.z - pos.z};
    return v.normalized();
  }
};

struct GLShader {};
struct LambertianPhongGLShader : public GLShader {
 private:
  float phongExp = 1;  // Phong Exponent

 public:
  void setPhoneExp(float p) { this->phongExp = p; }
  float getPhoneExp() const { return this->phongExp; }
  Color01 shade(GLLight* lighter, Eigen::Vector3f& uvNormal, Position3& pos, Eigen::Vector3f& uvEye,
                Color01& diffuse, Color01& ambient) {
    Eigen::Vector3f uvLight = lighter->uvLight(pos);
    Eigen::Vector3f uvHalf = (uvEye + uvLight).normalized();  // unit halfway vector
    float cp = 1 - std::max(diffuse.R, std::max(diffuse.G, diffuse.B));
    Color01 c;
    c.R = shadeComponent(diffuse.R, ambient.R, lighter->intensity.R, uvNormal, uvLight, cp, uvHalf,
                         phongExp);
    c.G = shadeComponent(diffuse.G, ambient.G, lighter->intensity.G, uvNormal, uvLight, cp, uvHalf,
                         phongExp);
    c.B = shadeComponent(diffuse.B, ambient.B, lighter->intensity.B, uvNormal, uvLight, cp, uvHalf,
                         phongExp);
    c.clamp();  // clamp 0 - 1
    return c;
  }
  inline float shadeComponent(float cr, float ca, float cl, Eigen::Vector3f& n, Eigen::Vector3f& l,
                              float cp, Eigen::Vector3f& h, float p) noexcept {
    return cr * (ca + cl * std::max(0.0f, n.dot(l))) + cl * cp * std::pow(h.dot(n), p);
  };
};
}  // namespace qtgl