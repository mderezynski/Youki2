INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/taglib
  ${TAGLIB_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
)

SET(taglib_plugin_mpc_SOURCES
  plugin.cc
)

ADD_LIBRARY(taglib_plugin_mpc MODULE ${taglib_plugin_mpc_SOURCES})

TARGET_LINK_LIBRARIES(taglib_plugin_mpc
  ${TAGLIB_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  mpx_common_reader
)

INSTALL(TARGETS taglib_plugin_mpc LIBRARY DESTINATION ${PLUGIN_DIR}/taglib)
