#pragma once

namespace qtgl {

struct MathUtils {
  constexpr static double PI = 3.1415926;
  inline static double toRadians(double d) { return d * (PI / 180); }
  inline static double toDegree(double r) { return r * (180 / PI); }
};

}  // namespace qtgl