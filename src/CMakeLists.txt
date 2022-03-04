add_executable(downpour
  main.cc
  assets.cc
  nine_patch.cc
)

add_dependencies(downpour embed_files)

set_target_properties(downpour PROPERTIES
  CXX_STANDARD 17
  CXX_EXTENSIONS OFF
)

target_link_libraries(downpour PRIVATE
  Box2D
  SFML::Audio
  SFML::Graphics
  # SFML::Main
  SFML::Network
  SFML::System
  SFML::Window
)

target_include_directories(downpour PRIVATE
  "${Box2D_SOURCE_DIR}"
  "${SFML_SOURCE_DIR}/include"
  "../include"
)
