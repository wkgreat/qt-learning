add_executable(eigen_test eigen_test.cpp)
target_link_libraries(eigen_test Eigen3::Eigen)

add_executable(opencv_test opencv_test.cpp)
target_link_libraries(opencv_test ${OpenCV_LIBS})