ADD_DEFINITIONS(-DENABLE_DEBUG=1)

INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIR}
  ${CURL_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
  ${GTKMM_INCLUDE_DIRS}
  ${SOUP_INCLUDE_DIRS}
  ${SIGX_INCLUDE_DIRS}
  ${XML_INCLUDE_DIRS}
)

SET(cppmod_lastfmscrobbler_SOURCES
  cppmod-scrobbler.cc
  cppmod-scrobbler.hh
  condition.cpp
  condition.h
  i-log.hh
  lastfmclient.cpp
  lastfmclient.h
  lastfmexceptions.h
  lastfmscrobbler.cpp
  lastfmscrobbler.h
  lastfmtypes.h
  log.cpp
  log.h
  md5.c
  md5.h
  mutex.cpp
  mutex.h
  nowplayinginfo.cpp
  nowplayinginfo.h
  scopedlock.cpp
  stringoperations.cpp
  scopedlock.h
  stringoperations.h
  submissioninfocollection.cpp
  submissioninfocollection.h
  submissioninfo.cpp
  submissioninfo.h
  thread.cpp
  thread.h
  urlclient.cpp
  urlclient.h
)

ADD_LIBRARY(cppmod-lastfmscrobbler MODULE ${cppmod_lastfmscrobbler_SOURCES})

TARGET_LINK_LIBRARIES(cppmod-lastfmscrobbler
  ${CURL_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  ${GTKMM_LIBRARIES}
  ${SOUP_LIBRARIES}
  ${SIGX_LIBRARIES}
  mconfig
)

INSTALL(TARGETS cppmod-lastfmscrobbler LIBRARY DESTINATION ${MPX_PLUGIN_DIR}/cppmod)
