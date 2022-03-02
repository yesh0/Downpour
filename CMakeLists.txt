cmake_minimum_required(VERSION 3.11...3.21)

project(liq
  VERSION 0.0.1
  DESCRIPTION "liq"
  LANGUAGES CXX
)

set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include(CTest)
include(FetchContent)
if(${CMAKE_VERSION} VERSION_LESS 3.14)
  include(cmake/add_FetchContent_MakeAvailable.cmake)
endif()

# liquidfun
FetchContent_Declare(
  liquidfun
  GIT_REPOSITORY https://github.com/yesh0/liquidfun.git
  GIT_TAG main
  SOURCE_SUBDIR liquidfun/Box2D
)
message(STATUS "Making available liquidfun...")
FetchContent_MakeAvailable(liquidfun)
# Adds liquidfun

# SDL2
FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.0.20)
message(STATUS "Making available SDL2...")
FetchContent_MakeAvailable(SDL2)
# Adds SDL2main


