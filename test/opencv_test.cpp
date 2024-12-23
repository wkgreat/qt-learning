#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

int main() {
  cv::Mat img =
      cv::imread("E:\\codes\\practice\\qt-learning\\data\\F16_Fighting_Falcon\\F16-lak.jpg");

  auto v = img.at<cv::Vec3b>(10, 10);

  int r = v[0];
  int g = v[1];
  int b = v[2];

  std::cout << r << " " << g << " " << b << std::endl;
  return 0;
}