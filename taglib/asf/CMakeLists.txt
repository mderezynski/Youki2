INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/taglib
  ${PROJECT_SOURCE_DIR}/taglib/gio
  ${TAGLIB_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
)

SET(taglib_plugin_asf_SOURCES
    asfattribute.cpp
    asfattribute.h
    asffile.cpp
    asffile.h
    asfpicture.cpp
    asfpicture.h
    asfproperties.cpp
    asfproperties.h
    asftag.cpp
    asftag.h
    plugin.cc
    resolver.cpp
    resolver.h
)

ADD_LIBRARY(taglib_plugin_asf MODULE ${taglib_plugin_asf_SOURCES})

TARGET_LINK_LIBRARIES(taglib_plugin_asf
  ${TAGLIB_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  mpx_common_reader
  mpx-taglib-gio
)

INSTALL(TARGETS taglib_plugin_asf LIBRARY DESTINATION ${MPX_PLUGIN_DIR}/taglib)

