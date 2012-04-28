INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIR}
  ${GLIBMM_INCLUDE_DIRS}
  ${GTKMM_INCLUDE_DIRS}
#  ${SIGX_INCLUDE_DIRS}
)

SET(mpx_view_components_SOVERSION 0)
SET(mpx_view_components_VERSION 0.0)

SET(mpx_view_components_SOURCES
  tagview.cc
)

ADD_LIBRARY(mpx-view-components SHARED ${mpx_view_components_SOURCES})

SET_TARGET_PROPERTIES(mpx-view-components
  PROPERTIES
  VERSION ${mpx_view_components_VERSION}
  SOVERSION ${mpx_view_components_SOVERSION}
)

TARGET_LINK_LIBRARIES(mpx-view-components
  ${GLIBMM_LIBRARIES}
  ${GTKMM_LIBRARIES}
#  ${SIGX_LIBRARIES}
  mpx-parsers
)

INSTALL(TARGETS mpx-view-components LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

INSTALL(
  FILES album-info-window.ui
        mb-import-album.ui
  DESTINATION ${MPX_PKGDATA_DIR}/ui
)
