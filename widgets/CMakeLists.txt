INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} # For generated headers
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include/mpx
  ${CLUTTERMM_INCLUDE_DIRS}
  ${GLIB_INCLUDE_DIRS}
  ${GTK_INCLUDE_DIRS}
  ${SEXYMM_INCLUDE_DIRS}
  ${SIGX_INCLUDE_DIRS}
  ${GIOMM_INCLUDE_DIRS}
  ${GLADEMM_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
  ${GTKMM_INCLUDE_DIRS}
  ${SEXYMM_INCLUDE_DIRS}
)

SET(mpx_widgets_SOVERSION 0)
SET(mpx_widgets_VERSION 0.0)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/widget-marshalers.c ${CMAKE_CURRENT_BINARY_DIR}/widget-marshalers.h
  COMMAND ${GLIB_GENMARSHAL} --body ${CMAKE_CURRENT_SOURCE_DIR}/widget-marshalers.list   > ${CMAKE_CURRENT_BINARY_DIR}/widget-marshalers.c
  COMMAND ${GLIB_GENMARSHAL} --header ${CMAKE_CURRENT_SOURCE_DIR}/widget-marshalers.list > ${CMAKE_CURRENT_BINARY_DIR}/widget-marshalers.h
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/widget-marshalers.list
)

SET(mpx_widgets_SOURCES
  youki-status-icon.c
  youki-status-icon.h
  youki-tray-icon.c
  youki-tray-icon.h
  youki-toggle-button.cc
  cairo-extensions.cc
  dialog-simple-entry.cc
  rounded-layout.cc
  rounded-frame.cc
  task-dialog.cc
  timed-confirmation.cc
  ${CMAKE_CURRENT_BINARY_DIR}/widget-marshalers.c
  ${CMAKE_CURRENT_BINARY_DIR}/widget-marshalers.h
)

ADD_LIBRARY(mpx-widgets SHARED ${mpx_widgets_SOURCES})

SET_TARGET_PROPERTIES(mpx-widgets
  PROPERTIES
  VERSION ${mpx_widgets_VERSION}
  SOVERSION ${mpx_widgets_SOVERSION}
)

TARGET_LINK_LIBRARIES(mpx-widgets
  ${CLUTTERMM_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  ${GTKMM_LIBRARIES}
  ${GLADEMM_LIBRARIES}
)

INSTALL(TARGETS mpx-widgets LIBRARY DESTINATION ${MPX_LIBDIR})
