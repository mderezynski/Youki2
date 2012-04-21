INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/taglib
  ${TAGLIB_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
)

SET(taglib_plugin_mp4_SOURCES
  plugin.cc
  aux.hh
  resolver.cpp
  resolver.h
  mp4atom.cpp
  mp4atom.h
  mp4file.cpp
  mp4file.h
  mp4item.cpp
  mp4item.h
  mp4properties.cpp
  mp4properties.h
  mp4tag.cpp
  mp4tag.h
  mp4coverart.cpp
  mp4coverart.h
)

ADD_LIBRARY(taglib_plugin_mp4 MODULE ${taglib_plugin_mp4_SOURCES})

TARGET_LINK_LIBRARIES(taglib_plugin_mp4
  ${TAGLIB_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  mpx_common_reader
)

INSTALL(TARGETS taglib_plugin_mp4 LIBRARY DESTINATION ${PLUGIN_DIR}/taglib)
