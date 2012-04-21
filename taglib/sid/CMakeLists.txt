INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
  ${TAGLIB_INCLUDE_DIRS}
  ${SIDPLAY_INCLUDE_DIR}
)

SET(taglib_plugin_sid_SOURCES
  plugin.cc
  resolver.cpp
  resolver.h
  sidfile.cpp
  sidfile.h
  sidproperties.cpp
  sidproperties.h
  sidtag.cpp
  sidtag.h
)

ADD_LIBRARY(taglib_plugin_sid MODULE ${taglib_plugin_sid_SOURCES})

TARGET_LINK_LIBRARIES(taglib_plugin_sid
  ${TAGLIB_LIBRARIES}
  ${SIDPLAY_LIBRARY}
)

INSTALL(TARGETS taglib_plugin_sid LIBRARY DESTINATION ${PLUGIN_DIR}/taglib)
