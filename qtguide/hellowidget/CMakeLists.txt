add_executable(hellowidget hellowidget.cpp hellowidget-main.cpp)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
target_link_libraries(hellowidget Qt5::Core Qt5::Widgets)