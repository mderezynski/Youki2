INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
)

SET(mpx_json_SOURCES
  json_reader.cpp
  json_value.cpp
  json_writer.cpp
)

ADD_LIBRARY(mpx-json STATIC ${mpx_json_SOURCES})
