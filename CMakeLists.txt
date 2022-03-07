cmake_minimum_required(VERSION 3.11...3.21)

project(Downpour
  VERSION 0.0.1
  DESCRIPTION "A game project utilizing liquidfun"
  LANGUAGES CXX
)

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

# SFML
FetchContent_Declare(
  SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 28e935673c16f9b2e7eefa46e59a1d5f7bb1fbb8
)
message(STATUS "Making available SFML...")
FetchContent_MakeAvailable(SFML)
# Adds SFML::*

# pugixml
FetchContent_Declare(
  pugixml
  GIT_REPOSITORY https://github.com/zeux/pugixml.git
  GIT_TAG v1.12.1
)
message(STATUS "Making available pugixml...")
FetchContent_MakeAvailable(pugixml)
# Adds pugixml::static

add_subdirectory(assets)

add_subdirectory(src)
