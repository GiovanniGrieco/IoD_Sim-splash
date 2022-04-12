cmake_minimum_required(VERSION 3.0.0)
if (WIN32)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake
  CACHE STRING "Vcpkg toolchain file")
endif (WIN32)

project(splash VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(splash splash.cc)

find_package(cxxopts CONFIG REQUIRED)
target_link_libraries(splash PRIVATE  cxxopts::cxxopts)

find_package(Clang CONFIG REQUIRED)
target_link_libraries(splash PRIVATE libclang)
include_directories(${CLANG_INCLUDE_DIRS})

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
