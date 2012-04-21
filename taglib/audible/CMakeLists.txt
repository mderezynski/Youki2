INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${TAGLIB_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
)

SET(taglib_plugin_audible_SOURCES 
  plugin.cc
  audibleproperties.cpp
  audibleproperties.h
  audibletag.cpp
  audibletag.h
  taglib_audiblefile.cpp
  taglib_audiblefile.h
  resolver.cpp
  resolver.h
)

ADD_LIBRARY(taglib_plugin_audible ${taglib_plugin_audible_SOURCES})

TARGET_LINK_LIBRARIES(taglib_plugin_audible
  ${TAGLIB_LIBARIES}
  ${GLIBMM_LIBARIES}
)

INSTALL(TARGETS taglib_plugin_audible LIBRARY DESTINATION ${PLUGIN_DIR}/taglib)
