set(CMAKE_INCLUDE_CURRENT_DIR true)
add_executable(qtglmain qtglmain.cpp mesh.cpp)
target_link_libraries(qtglmain Qt5::Core Qt5::Widgets Eigen3::Eigen)