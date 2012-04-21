INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/mpx
  ${Boost_INCLUDE_DIR}
  ${SOUP_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
)

SET(mpx_musicbrainz_SOVERSION 0)
SET(mpx_musicbrainz_VERSION 0.0)

SET(mpx_musicbrainz_SOURCES
  mb-libxml2-sax-release.cc
  mb-libxml2-sax-release.hh
  mb-libxml2-sax-release-list.cc
  mb-libxml2-sax-release-list.hh
  mb-libxml2-sax-tracks.cc
  mb-libxml2-sax-tracks.hh
  mb-utils.cc
  mb-utils.hh
  mbxml-v2.cc
  mbxml-v2.hh
  mbxml-types-v2.hh
)

ADD_LIBRARY(mpx-musicbrainz SHARED ${mpx_musicbrainz_SOURCES})

SET_TARGET_PROPERTIES(mpx-musicbrainz
  PROPERTIES
  VERSION ${mpx_musicbrainz_VERSION}
  SOVERSION ${mpx_musicbrainz_SOVERSION}
)

TARGET_LINK_LIBRARIES(mpx-musicbrainz
  ${Boost_FILESYSTEM_LIBRARY}
  ${GLIBMM_LIBRARIES}
  ${SOUP_LIBRARIES}
  mpx-parsers
)

INSTALL(TARGETS mpx-musicbrainz LIBRARY DESTINATION ${MPX_LIBDIR})
