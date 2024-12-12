#pragma once

#include "define.hpp"

namespace qtgl {

struct GLLight {
  Color01 intensity;
  virtual Eigen::Vector3d uvLight(Vertice& pos) = 0;  // l unit vector
};

struct DirectionalGLLight : public GLLight {
  Eigen::Vector3d d;  // direction
  Eigen::Vector3d uvLight(Vertice& pos) { return (d * -1).normalized(); }
};

struct PointGLLight : public GLLight {
  Vertice position;
  Eigen::Vector3d uvLight(Vertice& pos) { return (position - pos).head(3).normalized(); }
};

struct GLShader {
  virtual Color01 shade(GLLight* lighter, Eigen::Vector3d& uvNormal, Vertice& pos,
                        Eigen::Vector3d& uvEye, Color01& diffuse, Color01& ambient) = 0;
};
struct LambertianPhongGLShader : public GLShader {
 private:
  double phongExp = 1;  // Phong Exponent

 public:
  void setPhoneExp(double p) { this->phongExp = p; }
  double getPhoneExp() const { return this->phongExp; }
  Color01 shade(GLLight* lighter, Eigen::Vector3d& uvNormal, Vertice& pos, Eigen::Vector3d& uvEye,
                Color01& diffuse, Color01& ambient) {
    Eigen::Vector3d uvLight = lighter->uvLight(pos);
    Eigen::Vector3d uvHalf = (uvEye + uvLight).normalized();  // unit halfway vector
    double cp = 1 - std::max(diffuse.R, std::max(diffuse.G, diffuse.B));
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
  inline double shadeComponent(double cr, double ca, double cl, Eigen::Vector3d& n,
                               Eigen::Vector3d& l, double cp, Eigen::Vector3d& h,
                               double p) noexcept {
    return cr * (ca + cl * std::max(0.0, n.dot(l))) + cl * cp * std::pow(h.dot(n), p);
  };
};

}  // namespace qtgl