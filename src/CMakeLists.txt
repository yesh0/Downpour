add_executable(downpour
  main.cc
  assets.cc
  b2_tiled.cc
  nine_patch.cc
  particle_batch.cc
  tiled.cc
  tiled_world.cc
)

add_dependencies(downpour embed_files)

set_target_properties(downpour PROPERTIES
  CXX_STANDARD 17
  CXX_EXTENSIONS OFF
)

target_link_libraries(downpour PRIVATE
  Box2D
  pugixml::static
  SFML::Audio
  SFML::Graphics
  # SFML::Main
  SFML::Network
  SFML::System
  SFML::Window
)

target_include_directories(downpour PRIVATE
  "${Box2D_SOURCE_DIR}"
  "${pugixml_SOURCE_DIR}/src"
  "${SFML_SOURCE_DIR}/include"
  "../include"
)
