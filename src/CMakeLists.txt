IF(HAVE_HAL)
  SET(youki_bin_ADD_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/libhal++)
  SET(youki_bin_ADD_LIBRARIES halcc)
ENDIF(HAVE_HAL)

INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} # For generated files
  ${CMAKE_CURRENT_SOURCE_DIR} # FIXME: Get rid of cross header dependencies between /src and /include
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${ALSA_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIR}
  ${CAIROMM_INCLUDE_DIRS}
  ${DBUS_INCLUDE_DIRS}
  ${GDL_INCLUDE_DIRS}
  ${GIOMM_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
  ${GST_INCLUDE_DIRS}
  ${GTKMM_INCLUDE_DIRS}
  ${HAL_INCLUDE_DIRS}
  ${NM_INCLUDE_DIRS}
# ${PYGTK_INCLUDE_DIRS}
# ${PYTHON_INCLUDE_PATH}
  ${SOUP_INCLUDE_DIRS}
  ${SQLITE_INCLUDE_DIRS}
  ${STARTUP_NOTIFY_INCLUDE_DIRS}
  ${TAGLIB_INCLUDE_DIRS}
  ${WEBKIT_INCLUDE_DIRS}
  ${X11_X11_INCLUDE_PATH}
  ${XML_INCLUDE_DIRS}
  ${XPROTO_INCLUDE_DIRS}
  ${youki_bin_add_INCLUDE_DIRS}
)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mpx-mlibman-dbus-proxy.hh
  COMMAND ${DBUSBINDINGTOOL} ARGS "${PROJECT_SOURCE_DIR}/dbus/dbus-obj-MLIBMAN.xml" "--proxy=${CMAKE_CURRENT_BINARY_DIR}/mpx-mlibman-dbus-proxy.hh"
  MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/dbus/dbus-obj-MLIBMAN.xml
)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mpx-app-dbus-adaptor.hh
  COMMAND ${DBUSBINDINGTOOL} ARGS "${PROJECT_SOURCE_DIR}/dbus/dbus-obj-YOUKI.xml" "--adaptor=${CMAKE_CURRENT_BINARY_DIR}/mpx-app-dbus-adaptor.hh"
  MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/dbus/dbus-obj-YOUKI.xml
)

SET(youki_bin_SOURCES
  rm-covers.cc
  rm-covers-stores.cc
  glib-marshalers.h
  glib-marshalers.c
  kobo-position.hh
  kobo-position.cc
  kobo-titleinfo.hh
  kobo-titleinfo.cc
  youki-controller.hh
  youki-controller.cc
  kobo-main.hh
  kobo-main.cc
  kobo-volume.hh
  kobo-volume.cc
  youki-theme-engine.hh
  youki-theme-engine.cc
  youki-view-albums.hh
  youki-view-albums.cc
  youki-view-artist.hh
  youki-view-artist.cc
  ld-find-nearest.hh
  library.cc
  library.hh
  mpx-mlibman-dbus-proxy-actual.hh
  ${CMAKE_CURRENT_BINARY_DIR}/mpx-mlibman-dbus-proxy.hh
  ${CMAKE_CURRENT_BINARY_DIR}/mpx-app-dbus-adaptor.hh
  play.hh
  play.cc
  playlist-manager.hh
  playlist-manager.cc
  plugin.cc
#  plugin-loader-python.hh
#  plugin-loader-python.cc
#  plugin-types-python.cc
#  plugin-types-python.hh
  plugin-loader-cpp.cc
  plugin-loader-cpp.hh
  plugin-types-cpp.hh
  plugin-loader.hh
  plugin-manager-gui.hh
  plugin-manager-gui.cc
#  python.cc
#  pysigc.hh
#  gtkmm.cc
#  gtkmmmodule.cc
#  gtkmmmodule.h
  preferences.cc
  preferences.hh
  async_queue.hh
  resource_manager.hh
  rm-artist-images.hh
  rm-artist-images.cc
  splash-screen.hh
  splash-screen.cc
  webkit-album-info.hh
  webkit-album-info.cc
  stock.cc
  main.cc
)

# FIXME: Add definitions from: ${GLIB_CFLAGS} and ${GST_CFLAGS}

LIST(APPEND youki_bin_COMPILE_DEFINITIONS __USE_POSIX199309)

IF(HAVE_LINUX)
  LIST(APPEND youki_bin_COMPILE_DEFINITIONS _XOPEN_SOURCE=600)
ENDIF(HAVE_LINUX)

ADD_EXECUTABLE(youki-bin ${youki_bin_SOURCES})

SET_TARGET_PROPERTIES(youki-bin
  PROPERTIES
  COMPILE_DEFINITIONS "${youki_bin_COMPILE_DEFINITIONS}"
  LINK_FLAGS "-Wl,--export-dynamic"
)

TARGET_LINK_LIBRARIES(youki-bin
  ${ALSA_LIBRARIES}
  ${Boost_REGEX_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${CAIROMM_LIBRARIES}
  ${CLUTTERMM_LIBRARIES}
  ${CDIO_LIBRARIES}
  ${CDPARANOIA_LIBRARY}
  ${CURL_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${GDL_LIBRARIES}
  ${GIOMM_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  ${GST_LIBRARIES}
  ${GTKGLEXT_LIBRARIES}
  ${GTKMM_LIBRARIES}
  ${HAL_LIBRARIES}
  ${NM_LIBRARIES}
  ${LIBOFA_LIBRARIES}
#  ${PYGTK_LIBRARIES}
#  ${PYTHON_LIBRARIES}
  ${RSVG_LIBRARIES}
  ${SEXYMM_LIBRARIES}
  ${SM_LIBRARIES}
  ${SOUP_LIBRARIES}
  ${SQLITE_LIBRARIES}
  ${STARTUP_NOTIFY_LIBRARIES}
  ${TAGLIB_LIBRARIES}
  ${WEBKIT_LIBRARIES}
  ${XML_LIBRARIES}
  ${X11_X11_LIB}
  mconfig
  mpx-api
  mpx-sigx
  mpx-widgets
  mpx-musicbrainz
  mpx-view-components
  mpx-json
  ${youki_bin_ADD_LIBRARIES}
)

INSTALL(TARGETS youki-bin RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
