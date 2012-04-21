IF(HAVE_HAL)
  SET(youki_mlibman_bin_ADD_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/libhal++)
  SET(youki_mlibman_bin_ADD_LIBRARIES halcc)
ENDIF(HAVE_HAL)

INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}   # For generated headers
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${BOOST_CPPFLAGS}
  ${CAIROMM_INCLUDE_DIRS}
  ${DBUS_INCLUDE_DIRS}
  ${GIOMM_INCLUDE_DIRS}
  ${GLADEMM_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
  ${GST_INCLUDE_DIRS}
  ${GTKMM_INCLUDE_DIRS}
  ${HAL_INCLUDE_DIRS}
  ${NM_INCLUDE_DIRS}
  ${LIBOFA_INCLUDE_DIRS}
  ${SIGX_INCLUDE_DIRS}
  ${SOUP_INCLUDE_DIRS}
  ${SQLITE_INCLUDE_DIRS}
  ${TAGLIB_INCLUDE_DIRS}
  ${X11_X11_INCLUDE_PATH}
  ${XML_INCLUDE_DIRS}
  ${XPROTO_INCLUDE_DIRS}
  ${youki_mlibman_bin_ADD_INCLUDE_DIRS}
)

# FIXME: Check definitions in GLIB_INCLUDE_DIRS and GST_INCLUDE_DIRS
ADD_DEFINITIONS(-D__USE_POSIX199309)

IF(HAVE_LINUX)
  ADD_DEFINITIONS(-D_XOPEN_SOURCE=600)
ENDIF(HAVE_LINUX)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mpx-mlibman-dbus.hh
  COMMAND ${DBUSBINDINGTOOL} ARGS "${PROJECT_SOURCE_DIR}/dbus/dbus-obj-MLIBMAN.xml" "--adaptor=${CMAKE_CURRENT_BINARY_DIR}/mpx-mlibman-dbus.hh"
  MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/dbus/dbus-obj-MLIBMAN.xml
)

SET(youki_mlibman_bin_SOURCES
  main.cc
  import-folder.hh
  import-share.hh
  library-mlibman.hh
  library-scanner-thread-mlibman.hh
  mlibmanager.hh
  request-value.hh
  import-folder.cc
  import-share.cc
  library-mlibman.cc
  library-scanner-thread-mlibman.cc
  mlibmanager.cc
  request-value.cc
  ${CMAKE_CURRENT_BINARY_DIR}/mpx-mlibman-dbus.hh
)

ADD_EXECUTABLE(youki-mlibman-bin ${youki_mlibman_bin_SOURCES})

SET_TARGET_PROPERTIES(youki-mlibman-bin
  PROPERTIES
  LINK_FLAGS "-Wl,--export-dynamic"
)

TARGET_LINK_LIBRARIES(youki-mlibman-bin
  ${Boost_REGEX_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${CAIROMM_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${GIOMM_LIBRARIES}
  ${GLADEMM_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  ${GST_LIBRARIES}
  ${GTKMM_LIBRARIES}
  ${HAL_LIBRARIES}
  ${NM_LIBRARIES}
  ${LIBOFA_LIBRARIES}
  ${SIGX_LIBRARIES}
  ${SM_LIBRARIES}
  ${SOUP_LIBRARIES}
  ${SQLITE_LIBRARIES}
  ${TAGLIB_LIBRARIES}
  ${XML_LIBRARIES}
  ${X11_X11_LIB}
  mconfig
  mpx-api
  mpx-widgets
  mpx-musicbrainz
  ${youki_mlibman_bin_ADD_LIBRARIES}
)

INSTALL(TARGETS youki-mlibman-bin RUNTIME DESTINATION ${MPX_LIBEXECDIR})
