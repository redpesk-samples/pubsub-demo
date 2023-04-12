cmake_minimum_required (VERSION 3.13)

project (compare CXX)
find_package(PkgConfig REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library (compare MODULE compare.cpp)

pkg_check_modules (AFBBINDING REQUIRED afb-binding) 
include_directories(${AFBBINDING_INCLUDE_DIRS})
target_link_libraries (compare ${AFBBINDING_LDFLAGS})
