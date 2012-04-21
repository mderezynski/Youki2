INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${DBUS_INCLUDE_DIRS}
  ${GLIB_INCLUDE_DIRS}
  ${HAL_INCLUDE_DIRS}
  ${SIGC_INCLUDE_DIRS}
)

SET(halcc_ABI_VERSION 1.0)
SET(halcc_SOVERSION 3)
SET(halcc_VERSION 3.0.0)

SET(halcc_SOURCES
  context.cc
  device.cc
  drive.cc
  volume.cc
  util.cc
)

SET(halcc_COMMON_HEADERS
  context.hh
  device.hh
  exception.hh
  refptr.hh
  types.hh
  storage.hh
  drive.hh
  volume.hh
  hcc-macros.hh
  util.hh
  hal++.hh
)

ADD_LIBRARY(halcc SHARED ${halcc_SOURCES})

SET_TARGET_PROPERTIES(halcc
  PROPERTIES
  OUTPUT_NAME halcc-${halcc_ABI_VERSION}
  VERSION ${halcc_VERSION}
  SOVERSION ${halcc_SOVERSION}
)

TARGET_LINK_LIBRARIES(halcc
  ${DBUS_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${HAL_LIBRARIES}
  ${SIGC_LIBRARIES}
)

INSTALL(TARGETS halcc LIBRARY DESTINATION ${MPX_LIBDIR})

INSTALL(
  FILES ${halcc_COMMON_HEADERS}
  DESTINATION ${MPX_INCLUDEDIR}/hal++-1.0/hal++
)
