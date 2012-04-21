INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/taglib
  ${TAGLIB_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
)
  
SET(taglib_plugin_asf_SOURCES
  plugin.cc
  resolver.cpp
  resolver.h
  asfattribute.cpp
  asfattribute.h
  asffile.cpp
  asffile.h
  asfproperties.cpp
  asfproperties.h
  asftag.cpp
  asftag.h
)

ADD_LIBRARY(taglib_plugin_asf MODULE ${taglib_plugin_asf_SOURCES})

TARGET_LINK_LIBRARIES(taglib_plugin_asf
  ${TAGLIB_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  mpx_common_reader
)

INSTALL(TARGETS taglib_plugin_asf LIBRARY DESTINATION ${PLUGIN_DIR}/taglib)
