INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${GLIBMM_INCLUDE_DIRS}
  ${TAGLIB_INCLUDE_DIRS}
  ${MOD_INCLUDE_DIRS}
)

SET(taglib_plugin_mod_SOURCES
  plugin.cc
  modfile.cpp
  modfile.h
  modproperties.h
  modproperties.cpp
  modtag.cpp
  modtag.h
  resolver.cpp
  resolver.h
)

# Are these flags included? -fPIC -module
ADD_LIBRARY(taglib_plugin_mod MODULE ${taglib_plugin_mod_SOURCES})

TARGET_LINK_LIBRARIES(taglib_plugin_mod
  ${TAGLIB_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  ${MOD_LIBRARIES}
)

INSTALL(TARGETS taglib_plugin_moddir LIBRARY DESTINATION ${PLUGIN_DIR}/taglib)
