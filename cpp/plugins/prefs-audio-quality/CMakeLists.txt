INCLUDE_DIRECTORIES(cppmod_audioqualityprefs
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIR}
  ${GST_INCLUDE_DIRS}
  ${GLADEMM_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
  ${GTKMM_INCLUDE_DIRS}
)

SET(cppmod_audioqualityprefs
  prefs-audio-quality.hh
  prefs-audio-quality.cc
)

ADD_LIBRARY(cppmod-audioqualityprefs MODULE ${cppmod_audioqualityprefs})

TARGET_LINK_LIBRARIES(cppmod-audioqualityprefs
  ${GST_LIBRARIES}
  ${GLADEMM_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  ${GTKMM_LIBRARIES}
  mconfig
)

INSTALL(TARGETS cppmod-audioqualityprefs LIBRARY DESTINATION ${PLUGIN_DIR}/cppmod)
