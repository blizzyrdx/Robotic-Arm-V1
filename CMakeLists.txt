cmake_minimum_required(VERSION 3.22)

project(RobotArmController LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(
    LIBSERIALPORT
    REQUIRED
    IMPORTED_TARGET
    libserialport
)

add_executable(RobotArmController
    src/main.cpp
    src/GUI.cpp
    src/Serial.cpp
    src/Slider.cpp
    src/Button.cpp
)

target_include_directories(RobotArmController PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(RobotArmController PRIVATE
    SDL3::SDL3
    PkgConfig::LIBSERIALPORT
)