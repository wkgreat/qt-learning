#pragma once

#include "affineutils.hpp"
#include "define.hpp"

namespace qtgl {
/*
Heading : Yaw
Pitch : Pitch
Roll : Roll
*/
class GLCamera {
 private:
  float pos_x = 0;
  float pos_y = 0;
  float pos_z = 0;
  float heading = 0;
  float pitch = 0;
  float roll = 0;

 public:
  GLCamera() : pos_x(0), pos_y(0), pos_z(0), heading(0), pitch(0), roll(0) {}
  GLCamera(float pos_x, float pos_y, float pos_z, float heading, float pitch, float roll)
      : pos_x(pos_x), pos_y(pos_y), pos_z(pos_z), heading(heading), pitch(pitch), roll(roll) {}
  void setPosition(float x, float y, float z) {
    this->pos_x = x;
    this->pos_y = y;
    this->pos_z = z;
  }
  void setPosture(float heading, float pitch, float roll) {
    this->heading = heading;
    this->pitch = pitch;
    this->roll = roll;
  }
  void setPosX(float x) { this->pos_x = x; }
  void setPosY(float y) { this->pos_y = y; }
  void setPosZ(float z) { this->pos_z = z; }
  void setHeading(float heading) { this->heading = heading; }
  void setPitch(float pitch) { this->pitch = pitch; }
  void setRoll(float roll) { this->roll = roll; }
  float getPosX() const { return this->pos_x; }
  float getPosY() const { return this->pos_y; }
  float getPosZ() const { return this->pos_z; }
  float getHeading() const { return this->heading; }
  float getPitch() const { return this->pitch; }
  float getRool() const { return this->roll; }

  void lookAt(float fx, float fy, float fz, float tx, float ty, float tz) {
    // https://stackoverflow.com/a/33790309
    // TODO how to set z axis up
    Eigen::Vector3f d(tx - fx, ty - fy, tz - fz);
    d.normalize();
    this->pitch = asinf(-d[1]);
    this->heading = atan2f(d[0], d[2]);
    this->roll = 0;
    this->setPosition(fx, fy, fz);
  }

  Eigen::Matrix4f viewMatrix() {
    Eigen::Matrix4f eyeMtx = Eigen::Matrix4f::Identity();
    Vertices rotateMtx = static_cast<Vertices>(eyeMtx);
    rotateMtx = AffineUtils::rotate_y(rotateMtx, -heading);
    rotateMtx = AffineUtils::rotate_x(rotateMtx, -pitch);
    rotateMtx = AffineUtils::rotate_z(rotateMtx, -roll);
    Vertices translateMtx = static_cast<Vertices>(eyeMtx);
    translateMtx = AffineUtils::translate(translateMtx, -pos_x, -pos_y, -pos_z);
    Eigen::Matrix4f viewMtx =
        static_cast<Eigen::Matrix4f>(translateMtx) * static_cast<Eigen::Matrix4f>(rotateMtx);
    return viewMtx;
  }
};
}  // namespace qtgl