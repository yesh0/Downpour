# Seems to be modified from CMake's FetchContent_MakeAvailable

# # Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# # file CMake-Copyright.txt or https://cmake.org/licensing for details.

macro(FetchContent_MakeAvailable NAME)
  FetchContent_GetProperties(${NAME})
  if(NOT ${NAME}_POPULATED)
    FetchContent_Populate(${NAME})
    add_subdirectory(${${NAME}_SOURCE_DIR} ${${NAME}_BINARY_DIR})
  endif()
endmacro()
