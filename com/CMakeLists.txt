INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIR}
  ${GLADEMM_INCLUDE_DIRS}
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
  ${GLADEMM_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  ${GTKMM_LIBRARIES}
#  ${SIGX_LIBRARIES}
  mpx-parsers
)

INSTALL(TARGETS mpx-view-components LIBRARY DESTINATION ${MPX_LIBDIR})

INSTALL(
  FILES album-info-window.glade
        mb-import-album.glade
  DESTINATION ${MPX_PKGDATADIR}/glade
)
