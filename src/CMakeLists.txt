add_executable(downpour
  main.cc
)

target_link_libraries(downpour PRIVATE
  Box2D
  SDL2main
  SDL2-static
)

target_include_directories(downpour PRIVATE
  "${SDL2_SOURCE_DIR}/include"
  "${Box2D_SOURCE_DIR}"
)
