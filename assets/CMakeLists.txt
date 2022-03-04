set(EMBED_EXPORT ${CMAKE_BINARY_DIR}/ImportEmbedder.cmake)
set(EMBEDDER file_embedder)
if(CMAKE_CROSSCOMPILING)
  set(IMPORT_EMBEDDER "IMPORTFILE-NOTFOUND" CACHE FILEPATH ${EMBED_EXPORT})
  include(${IMPORT_EMBEDDER})
endif()

if(NOT CMAKE_CROSSCOMPILING)
  add_executable(${EMBEDDER} file_embedder.cc)
  set_target_properties(${EMBEDDER} PROPERTIES
    CXX_STANDARD 17
  )
endif()

set(EMBED_OUTPUT "${Downpour_SOURCE_DIR}/include/builtin_assets.cc")
list(APPEND EMBED_FILES
  ${Downpour_SOURCE_DIR}/assets/embedded
)
add_custom_command(
  OUTPUT ${EMBED_OUTPUT}
  COMMAND file_embedder ${EMBED_OUTPUT} ${EMBED_FILES}
  DEPENDS ${EMBED_FILES} ${EMBEDDER}
)
add_custom_target(
  embed_files
  DEPENDS ${EMBED_OUTPUT} ${EMBEDDER}
)

if(NOT CMAKE_CROSSCOMPILING)
  export(TARGETS file_embedder FILE ${EMBED_EXPORT})
endif()
