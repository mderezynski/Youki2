INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/taglib
  ${TAGLIB_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
)

SET(taglib_plugin_flac_SOURCES
  plugin.cc
)

ADD_LIBRARY(taglib_plugin_flac MODULE ${taglib_plugin_flac_SOURCES})

TARGET_LINK_LIBRARIES(taglib_plugin_flac
  ${TAGLIB_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  mpx_id3v2_reader
  mpx_xiph_reader
  mpx_common_reader
)

INSTALL(TARGETS taglib_plugin_flac LIBRARY DESTINATION ${PLUGIN_DIR}/taglib)
