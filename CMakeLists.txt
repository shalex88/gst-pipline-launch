include(cmake/EnableVcpkg.cmake)
cmake_minimum_required(VERSION 3.22)

project(gst-pipline-launch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_search_module(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)

include_directories(
    include
    source
)

file(GLOB_RECURSE sourceFiles CONFIGURE_DEPENDS "source/*.cpp")

add_executable(${PROJECT_NAME} ${sourceFiles})

target_link_libraries(${PROJECT_NAME} PRIVATE
    PkgConfig::GSTREAMER
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    cxxopts::cxxopts
)