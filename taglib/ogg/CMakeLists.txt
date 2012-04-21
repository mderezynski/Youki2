INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/taglib
  ${TAGLIB_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
)

SET(taglib_plugin_ogg_SOURCES
  plugin.cc
)

ADD_LIBRARY(taglib_plugin_ogg MODULE ${taglib_plugin_ogg_SOURCES})

TARGET_LINK_LIBRARIES(taglib_plugin_ogg
  ${TAGLIB_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  mpx_xiph_reader
  mpx_common_reader
)

INSTALL(TARGETS taglib_plugin_ogg LIBRARY DESTINATION ${PLUGIN_DIR}/taglib)
