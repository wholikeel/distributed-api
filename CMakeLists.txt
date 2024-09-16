cmake_minimum_required(VERSION 3.29)

project(
    "code-jam-dapi"
    VERSION 0.0.1
    DESCRIPTION "API Code Jame using a DAPI"
    LANGUAGES CXX
)

include(CTest)
include(GoogleTest)


enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory("libpeer")
add_subdirectory("src")
add_subdirectory("tests")

install(
  TARGETS geocli peer
  EXPORT ${PROJECT_NAME}_Targets
  INCLUDES
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})



