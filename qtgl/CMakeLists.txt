add_executable(affine2d affine2d.cpp)
target_link_libraries(affine2d Qt5::Core Qt5::Widgets Eigen3::Eigen)

add_executable(affine3d affine3d.cpp)
target_link_libraries(affine3d Qt5::Core Qt5::Widgets Eigen3::Eigen)