INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
  ${XML_INCLUDE_DIRS}
  ${XERCESC_INCLUDE_DIR}
)

SET(mpx_parsers_SOVERSION 0)
SET(mpx_parsers_VERSION 0.0)

SET(mpx_parsers_SOURCES
  mb-artist-basic-1.0.cxx
  mb-artist-basic-1.0.hxx
  xsd-artist-similar-2.0.cxx
  xsd-artist-similar-2.0.hxx
  xsd-artist-info-2.0.cxx
  xsd-artist-info-2.0.hxx
  xsd-artist-toptracks-2.0.cxx
  xsd-artist-toptracks-2.0.hxx
  xsd-topalbums-2.0.cxx
  xsd-topalbums-2.0.hxx
  xsd-topartists-2.0.cxx
  xsd-topartists-2.0.hxx
  xspf.cxx
  xspf.hxx
)

ADD_LIBRARY(mpx-parsers SHARED ${mpx_parsers_SOURCES})

SET_TARGET_PROPERTIES(mpx-parsers
  PROPERTIES
  VERSION ${mpx_parsers_VERSION}
  SOVERSION ${mpx_parsers_SOVERSION}
)

TARGET_LINK_LIBRARIES(mpx-parsers
  ${XERCESC_LIBRARY}
)

INSTALL(TARGETS mpx-parsers LIBRARY DESTINATION ${MPX_LIBDIR})
