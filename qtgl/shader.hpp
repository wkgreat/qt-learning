#pragma once

#include <cmath>
#include "define.hpp"
#include "material.hpp"

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
  virtual Color01 shade(std::vector<GLLight*> lights, Color01 ambient, GLMaterial* material,
                        Vertice& position, Eigen::Vector3d& uvNormal, Eigen::Vector3d& uvView,
                        TexCoord* coord) = 0;
};

struct LambertianGLShader : public GLShader {
  Color01 shade(std::vector<GLLight*> lights, Color01 ambient, GLMaterial* material,
                Vertice& position, Eigen::Vector3d& uvNormal, Eigen::Vector3d& uvView,
                TexCoord* coord) {
    Eigen::Vector3d uvLight;
    Color01 r(0, 0, 0, 0);
    r = r + ambient.cwiseProduct(material->getAmbient(coord));
    Color01 p(0, 0, 0, 0);
    for (GLLight* light : lights) {
      uvLight = light->uvLight(position);
      p = p + (uvLight.dot(uvNormal)) * light->intensity;
    }
    r = r + p.cwiseProduct(material->getDiffuse(coord));
    return r;
  }
};
struct LambertialBlinnPhongGLShader : public GLShader {
  Color01 shade(std::vector<GLLight*> lights, Color01 ambient, GLMaterial* material,
                Vertice& position, Eigen::Vector3d& uvNormal, Eigen::Vector3d& uvView,
                TexCoord* coord) {
    Eigen::Vector3d uvLight;
    Eigen::Vector3d uvHalf;
    Color01 r(0, 0, 0, 0);
    r = r + ambient.cwiseProduct(material->getAmbient(coord));
    Color01 d(0, 0, 0, 0);
    Color01 s(0, 0, 0, 0);
    for (GLLight* light : lights) {
      uvLight = light->uvLight(position);
      uvHalf = (uvView + uvLight).normalized();
      d = d + (uvLight.dot(uvNormal)) * light->intensity;
      s = s + std::pow(uvHalf.dot(uvNormal), material->getSpecularHighlight()) * light->intensity;
    }
    r = r + d.cwiseProduct(material->getDiffuse(coord));
    r = r + s.cwiseProduct(material->getSpecular());
    return r;
  }
};

// struct LambertianPhongGLShader : public GLShader {
//  private:
//   double phongExp = 1;             // Phong Exponent
//   Color01 ambient = {0, 0, 0, 0};  // 环境光

//  public:
//   LambertianPhongGLShader() {}
//   LambertianPhongGLShader(double phongExp) { this->phongExp = phongExp; }
//   LambertianPhongGLShader(double phongExp, Color01 ambient) {
//     this->phongExp = phongExp;
//     this->ambient = ambient;
//   }
//   void setPhoneExp(double p) { this->phongExp = p; }
//   double getPhoneExp() const { return this->phongExp; }
//   void setAmbient(Color01 c) { this->ambient = c; }
//   Color01 getAmbient() const { return this->ambient; }
//   Color01 shade(GLLight* lighter, Eigen::Vector3d& uvNormal, Vertice& pos, Eigen::Vector3d&
//   uvEye,
//                 Color01& diffuse) {
//     Eigen::Vector3d uvLight = lighter->uvLight(pos);
//     Eigen::Vector3d uvHalf = (uvEye + uvLight).normalized();  // unit halfway vector
//     double cp = 1 - std::max(diffuse.R, std::max(diffuse.G, diffuse.B));
//     Color01 c;
//     c.R = shadeComponent(diffuse.R, ambient.R, lighter->intensity.R, uvNormal, uvLight, cp,
//     uvHalf,
//                          phongExp);
//     c.G = shadeComponent(diffuse.G, ambient.G, lighter->intensity.G, uvNormal, uvLight, cp,
//     uvHalf,
//                          phongExp);
//     c.B = shadeComponent(diffuse.B, ambient.B, lighter->intensity.B, uvNormal, uvLight, cp,
//     uvHalf,
//                          phongExp);
//     c.clamp();  // clamp 0 - 1
//     return c;
//   }
//   inline double shadeComponent(double cr, double ca, double cl, Eigen::Vector3d& n,
//                                Eigen::Vector3d& l, double cp, Eigen::Vector3d& h,
//                                double p) noexcept {
//     return cr * (ca + cl * std::max(0.0, n.dot(l))) + cl * cp * std::pow(h.dot(n), p);
//   };
// };

}  // namespace qtgl