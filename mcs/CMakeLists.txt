INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${XML_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
  ${GTKMM_INCLUDE_DIRS}
)

SET(mconfig_SOURCES
  base.h
  gtk-bind.h	  
  key.h
  key.cpp
  mcs.cpp
  mcs.h
  mcs-gtk-bind.cpp
  subscriber.cpp
  subscriber.h
  types.h
)

ADD_LIBRARY(mconfig STATIC ${mconfig_SOURCES})

SET_TARGET_PROPERTIES(mconfig
  PROPERTIES COMPILE_FLAGS -fPIC
)

TARGET_LINK_LIBRARIES(mconfig
  ${GLIB_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  ${GTKMM_LIBRARIES}
  ${XML_LIBRARIES}
)
