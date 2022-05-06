add_executable(downpour
  main.cc
  animated_sprite.cc
  assets.cc
  b2_tiled.cc
  level.cc
  level_base.cc
  levels.cc
  nine_patch.cc
  particle_batch.cc
  stage.cc
  tiled.cc
  tiled_world.cc
)

if(BUNDLE_DOWNPOUR)
  add_dependencies(downpour embed_files)
  add_compile_definitions(BUNDLE_DOWNPOUR)
endif()

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
