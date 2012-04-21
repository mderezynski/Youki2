INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/libhal++
  ${Boost_INCLUDE_DIR}
  ${GLADEMM_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
  ${GTKMM_INCLUDE_DIRS}
  ${HAL_INCLUDE_DIRS}
)

SET(cppmod_libraryprefs_SOURCES
  prefs-library.cc
  prefs-library.hh
)

ADD_LIBRARY(cppmod-libraryprefs MODULE ${cppmod_libraryprefs_SOURCES})

TARGET_LINK_LIBRARIES(cppmod-libraryprefs
  ${GLADEMM_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  ${GTKMM_LIBRARIES}
  ${HAL_LIBRARIES}
  mconfig
)

INSTALL(TARGETS cppmod-libraryprefs LIBRARY DESTINATION ${PLUGIN_DIR}/cppmod)
