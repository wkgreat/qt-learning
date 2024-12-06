#pragma once

namespace qtgl {

struct MathUtils {
  constexpr static float PI = 3.1415926f;
  inline static float toRadians(float d) { return d * (PI / 180); }
  inline static float toDegree(float r) { return r * (180 / PI); }
};

}  // namespace qtgl