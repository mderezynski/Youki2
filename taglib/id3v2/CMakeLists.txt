INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/taglib/gio
  ${TAGLIB_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
)

SET(mpx_id3v2_reader_SOURCES
  reader.cc
  reader.hh
)

ADD_LIBRARY(mpx_id3v2_reader STATIC ${mpx_id3v2_reader_SOURCES})

SET_TARGET_PROPERTIES(mpx_id3v2_reader
  PROPERTIES COMPILE_FLAGS -fPIC
)

TARGET_LINK_LIBRARIES(mpx_id3v2_reader
  ${TAGLIB_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  mpx-taglib-gio
)
