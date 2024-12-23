#pragma once
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/eigen.hpp>
#include <opencv2/imgcodecs.hpp>
#include "define.hpp"

namespace qtgl {

class GLTexture {
 public:
  GLTexture() = default;
  virtual ~GLTexture() = default;
  virtual Color01 sample(TexCoord& coord) = 0;
};

class InterpolateGLTexture : public GLTexture {
 public:
  cv::Mat mat;
  InterpolateGLTexture() = default;
  ~InterpolateGLTexture() = default;
  InterpolateGLTexture(std::string mapref) {
    cv::Mat img = cv::imread(mapref);
    if (img.empty()) {
      std::cerr << "Error: cannot load image " << mapref << std::endl;
      return;
    }
    this->mat = img;
  };
  Color01 sample(TexCoord& coord) {
    int i = round(mat.rows - coord[1] * mat.rows);
    int j = round(coord[0] * mat.cols);  // TODO y轴反转？
    i = MAX(0, i);                       // TODO 临时添加
    j = MAX(0, j);                       // TODO 临时添加
    cv::Vec3b v = mat.at<cv::Vec3b>(i % mat.rows, j % mat.cols);
    Color01 c = {v[0] / 255.0, v[1] / 255.0, v[2] / 255.0, 1};
    return c;
  }
};

}  // namespace qtgl