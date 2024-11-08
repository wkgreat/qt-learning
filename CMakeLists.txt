cmake_minimum_required(VERSION 3.10)
project(qt-learning VERSION 0.0.1)
include(E:/vcpkg/scripts/buildsystems/vcpkg.cmake)
find_package(Qt5 REQUIRED Core Widgets)

add_subdirectory(qtguide)