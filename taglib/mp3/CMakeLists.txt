INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/taglib
  ${PROJECT_SOURCE_DIR}/taglib/gio
  ${TAGLIB_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
)

SET(taglib_plugin_mp3_SOURCES
  plugin.cc
)

ADD_LIBRARY(taglib_plugin_mp3 MODULE ${taglib_plugin_mp3_SOURCES})

TARGET_LINK_LIBRARIES(taglib_plugin_mp3
  ${TAGLIB_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  mpx_id3v2_reader
  mpx_common_reader
  mpx-taglib-gio
)

INSTALL(TARGETS taglib_plugin_mp3 LIBRARY DESTINATION ${MPX_PLUGIN_DIR}/taglib)
