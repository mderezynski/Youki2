CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(youki)

SET(PACKAGE           "youki")
SET(PACKAGE_VERSION   "0.2")
SET(PACKAGE_NAME      "youki")
SET(PACKAGE_BUGREPORT "http://mpx.backtrace.info/report")
SET(PACKAGE_STRING    "${PACKAGE_NAME} ${PACKAGE_VERSION}")
SET(PACKAGE_TARNAME   "youki")

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

INCLUDE(GNUInstallDirs)
SET(MPX_PKGDATA_DIR      ${CMAKE_INSTALL_DATADIR}/${PACKAGE})
SET(MPX_PKGDATA_DIR_FULL ${CMAKE_INSTALL_FULL_DATADIR}/${PACKAGE})
SET(MPX_PLUGIN_DIR       ${CMAKE_INSTALL_LIBDIR}/${PACKAGE}/plugins)
SET(MPX_PLUGIN_DIR_FULL  ${CMAKE_INSTALL_FULL_LIBDIR}/${PACKAGE}/plugins)
SET(MPX_PKGDATA_DIR      ${CMAKE_INSTALL_DATADIR}/${PACKAGE})
SET(MPX_PKGDATA_DIR_FULL ${CMAKE_INSTALL_FULL_DATADIR}/${PACKAGE})

INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckLibraryExists)
INCLUDE(FeatureSummary)

CHECK_INCLUDE_FILES(
  argz.h
  arpa/inet.h
  fcntl.h
  float.h
  inttypes.h
  langinfo.h
  libintl.h
  limits.h
  locale.h
  malloc.h
  netdb.h
  netinet/in.h
  stddef.h
  stdint.h
  stdio_ext.h
  stdlib.h
  string.h
  strings.h
  sys/param.h
  sys/socket.h
  sys/time.h
  unistd.h
  wchar.h
  HAVE_MISC_HEADERS
)

CHECK_LIBRARY_EXISTS(socket connect "" HAVE_SOCKET_LIB)
CHECK_LIBRARY_EXISTS(nsl getrpcbyname "" HAVE_NSL)

OPTION(ENABLE_NLS "Enable localization" yes)
IF(ENABLE_NLS)
  FIND_PACKAGE(Gettext REQUIRED)
  SET(HAVE_GETTEXT ${GETTEXT_FOUND})

  ADD_SUBDIRECTORY(po)
ENDIF()

# pkg-config
FIND_PACKAGE(PkgConfig REQUIRED)

# Boost
FIND_PACKAGE(Boost 1.33.1 COMPONENTS filesystem regex REQUIRED)

# X libraries
FIND_PACKAGE(X11 REQUIRED)

PKG_CHECK_MODULES(SIGC       REQUIRED sigc++-2.0)
PKG_CHECK_MODULES(GLIB       REQUIRED glib-2.0>=2.10.0 gobject-2.0 gthread-2.0 gmodule-2.0)
PKG_CHECK_MODULES(GLIBMM     REQUIRED glibmm-2.4>=2.8.3)
PKG_CHECK_MODULES(GIOMM      REQUIRED giomm-2.4)
PKG_CHECK_MODULES(GTK        REQUIRED gtk+-3.0)
PKG_CHECK_MODULES(GTKMM      REQUIRED gtkmm-3.0)
PKG_CHECK_MODULES(CAIROMM    REQUIRED cairomm-1.0>=0.6.0)
PKG_CHECK_MODULES(XML        REQUIRED libxml-2.0>=2.6.1)
PKG_CHECK_MODULES(SOUP       REQUIRED libsoup-2.4>=2.2.10)
PKG_CHECK_MODULES(SQLITE     REQUIRED sqlite3>=3.4)
PKG_CHECK_MODULES(NOTIFY     REQUIRED libnotify)
PKG_CHECK_MODULES(WEBKIT     REQUIRED webkitgtk-3.0)

# Python
# FIND_PACKAGE(PythonLibs REQUIRED)

# PyGTK
#PKG_CHECK_MODULES(PYGTK REQUIRED pygtk-2.0>=2.12.0)
#IF(NOT DEFINED PYGTK_DEFSDIR)
#  EXECUTE_PROCESS(
#    COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=defsdir pygtk-2.0
#    OUTPUT_VARIABLE PYGTK_DEFSDIR
#    OUTPUT_STRIP_TRAILING_WHITESPACE
#  )
#ENDIF()

# Network Manager
PKG_CHECK_MODULES(NM REQUIRED libnm-glib>=0.9.4.0)

# Xerces C
FIND_PACKAGE(XercesC REQUIRED)

# GStreamer
PKG_CHECK_MODULES(GST              REQUIRED gstreamer-0.10>=0.10.14 gstreamer-base-0.10>=0.10.14)
PKG_CHECK_MODULES(GST_PLUGINS_BASE REQUIRED gstreamer-plugins-base-0.10>=0.10.9)

# D-Bus

PKG_CHECK_MODULES(DBUS REQUIRED dbus-c++-1 dbus-c++-glib-1 dbus-glib-1)
IF(NOT DEFINED DBUS_SERVICES_DIR)
  EXECUTE_PROCESS(
    COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=prefix dbus-1
    OUTPUT_VARIABLE DEFAULT_DBUS_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  SET(DBUS_SERVICES_DIR "${DEFAULT_DBUS_PREFIX}/share/dbus-1/services")
ENDIF()

FIND_PROGRAM(DBUSBINDINGTOOL "dbusxx-xml2cpp")
IF(NOT DBUSBINDINGTOOL)
  MESSAGE(ERROR "Cannot find dbusxx-xml2cpp")
ENDIF()
IF(NOT DEFINED GLIB_GENMARSHAL)
  EXECUTE_PROCESS(
    COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=glib_genmarshal glib-2.0
    OUTPUT_VARIABLE GLIB_GENMARSHAL
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
ENDIF()

# HAL
OPTION(ENABLE_HAL "Enable storage management with HAL" yes)

IF(ENABLE_HAL)
  PKG_CHECK_MODULES(HAL REQUIRED hal>=0.5.8.1 hal-storage>=0.5.8.1)
  SET(HAVE_HAL ${HAL_FOUND})
ENDIF()

# SMlib/ICE
OPTION(ENABLE_SM "Enable session management support with SMlib" yes)
IF(ENABLE_SM)
  FIND_PACKAGE(SMlib REQUIRED)
  SET(HAVE_SM ${SM_FOUND})
ENDIF()

# Startup Notifications
# OPTION(ENABLE_SN "Enable Startup Notification" yes)
# IF(ENABLE_SN)
#  PKG_CHECK_MODULES(STARTUP_NOTIFY REQUIRED libstartup-notification-1.0>=0.8)
#  SET(HAVE_STARTUP_NOTIFICATION ${STARTUP_NOTIFY_FOUND})
# ENDIF()

# Default audio devices
SET(DEFAULT_SINK "autoaudiosink")
SET(DEFAULT_DEVICE "/dev/dsp")
SET(DEFAULT_DEVICE_OSS  "/dev/dsp0")
SET(DEFAULT_DEVICE_ALSA "default")
SET(DEFAULT_DEVICE_SUN  "/dev/audio")
SET(DEFAULT_DEVICE_ESD  "localhost")

# Platform defines

IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
  SET(HAVE_LINUX yes)
  PKG_CHECK_MODULES(ALSA REQUIRED alsa>=1.0.9)
  SET(DEFAULT_SINK "alsasink")
  SET(DEFAULT_DEVICE "default")
  SET(HAVE_ALSA yes)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  SET(HAVE_FREEBSD yes)
  SET(DEFAULT_SINK "osssink")
  SET(DEFAULT_DEVICE "/dev/dsp")
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "DragonFly")
  SET(HAVE_DRAGONFLYBSD yes)
  SET(DEFAULT_SINK "osssink")
  SET(DEFAULT_DEVICE "/dev/dsp")
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "NetBSD")
  SET(HAVE_NETBSD yes)
  SET(DEFAULT_SINK "osssink")
  SET(DEFAULT_DEVICE "/dev/audio")
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "SunOS")
  SET(HAVE_SUN yes)
  SET(DEFAULT_SINK "sunaudiosink")
  SET(DEFAULT_DEVICE "/dev/audio")
ENDIF()

# Multimedia keys
PKG_CHECK_MODULES(XPROTO REQUIRED xproto>=7.0.10)

SET(MPX_DBUS_SERVICE_ "\"org.mpris.mpx\"")
SET(MPX_DBUS_INTERFACE__MPRIS_ "\"org.freedesktop.MediaPlayer\"")
SET(MPX_DBUS_PATH__MPRIS_ROOT_ "\"/\"")
SET(MPX_DBUS_PATH__MPRIS_PLAYER_ "\"/Player\"")
SET(MPX_DBUS_PATH__MPRIS_TRACKLIST_ "\"/TrackList\"")

SET(MPX_DBUS_INTERFACE__MPX_ "\"info.backtrace.mpx\"")
SET(MPX_DBUS_PATH__MPX_ "\"/MPX\"")

SET(PLUGIN_VERSION 10)

# MCS stubs
SET(HAVE_BMP yes)
SET(HAVE_GTK yes)
SET(HAVE_XML yes)

# Standard flags
# NOTE: We cannot use -pedantic with g++ because long long (needed
# for 64-bit integers on 32-bit platforms) is not defined in C++98
SET(COMMON_FLAGS "-Wall -D_FORTIFY_SOURCE=2")
IF(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  SET(COMPILER_FLAGS "-pipe -fno-strict-aliasing -fmessage-length=0")
  SET(C_LANG_FLAGS   "-std=c99 -pedantic")
  SET(CXX_LANG_FLAGS "-std=c++0x -fpermissive")
ELSEIF(CMAKE_C_COMPILER_ID STREQUAL "Clang")
  SET(COMPILER_FLAGS "-Wno-mismatched-tags")
  SET(C_LANG_FLAGS   "-pedantic") # Clang defaults to C99
  SET(CXX_LANG_FLAGS "")
ENDIF()

SET(CMAKE_C_FLAGS   "${COMMON_FLAGS} ${C_LANG_FLAGS} ${COMPILER_FLAGS} -ggdb3 -pg -fprofile-arcs -ftest-coverage") 
SET(CMAKE_CXX_FLAGS "${COMMON_FLAGS} ${CXX_LANG_FLAGS} ${COMPILER_FLAGS} -ggdb3 -pg -fprofile-arcs -ftest-coverage") 

# Profile build C/C++ flags
SET(CMAKE_C_FLAGS_PROFILE   "-ggdb3 -pg -fprofile-arcs -ftest-coverage")
SET(CMAKE_CXX_FLAGS_PROFILE "-ggdb3 -pg -fprofile-arcs -ftest-coverage")

# Debug build C/C++ flags
SET(CMAKE_C_FLAGS_DEBUG   "-ggdb3 -fno-inline")
SET(CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -fno-inline")

# Release build C/C++ flags
SET(CMAKE_C_FLAGS_RELEASE   "-O2")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")

# Add RPATHs where necessary
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_FULL_LIBDIR}" IS_SYSTEM_DIR)
IF(IS_SYSTEM_DIR STREQUAL "-1")
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
ENDIF()

ADD_DEFINITIONS(-DHAVE_CONFIG_H)

LINK_DIRECTORIES(
  ${ALSA_LIBRARY_DIRS}
  ${Boost_LIBRARY_DIRS}
  ${CLUTTERMM_LIBRARY_DIRS}
  ${CURL_LIBRARY_DIRS}
  ${DBUS_LIBRARY_DIRS}
  ${CAIROMM_LIBRARY_DIRS}
  ${GIOMM_LIBRARY_DIRS}
  ${GLIB_LIBRARY_DIRS}
  ${GLIBMM_LIBRARY_DIRS}
  ${GST_LIBRARY_DIRS}
  ${GST_PLUGINS_BASE_LIBRARY_DIRS}
  ${GTK_LIBRARY_DIRS}
  ${GTKMM_LIBRARY_DIRS}
  ${HAL_LIBRARY_DIRS}
  ${MOD_LIBRARY_DIRS}
  ${NM_LIBRARY_DIRS}
  ${LIBOFA_LIBRARY_DIRS}
#  ${PYGTK_LIBRARY_DIRS}
  ${SIGC_LIBRARY_DIRS}
  ${SOUP_LIBRARY_DIRS}
  ${SQLITE_LIBRARY_DIRS}
  ${STARTUP_NOTIFY_LIBRARY_DIRS}
  ${XML_LIBRARY_DIRS}
)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/mpx.1.in ${PROJECT_BINARY_DIR}/mpx.1)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/mpx.pc.in ${PROJECT_BINARY_DIR}/mpx.pc)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/info.backtrace.youki.app.service.in ${PROJECT_BINARY_DIR}/info.backtrace.youki.app.service)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/info.backtrace.youki.mlibman.service.in ${PROJECT_BINARY_DIR}/info.backtrace.youki.mlibman.service)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/info.backtrace.youki.sentinel.service.in ${PROJECT_BINARY_DIR}/info.backtrace.youki.sentinel.service)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h)

ADD_SUBDIRECTORY(xmlcpp)
ADD_SUBDIRECTORY(musicbrainz)
ADD_SUBDIRECTORY(mcs)
ADD_SUBDIRECTORY(remote)
ADD_SUBDIRECTORY(sentinel)
ADD_SUBDIRECTORY(widgets)
ADD_SUBDIRECTORY(api)
ADD_SUBDIRECTORY(sigx)
ADD_SUBDIRECTORY(taglib)
ADD_SUBDIRECTORY(com)
ADD_SUBDIRECTORY(cpp)
ADD_SUBDIRECTORY(desktop)
ADD_SUBDIRECTORY(ui)
ADD_SUBDIRECTORY(icons)
ADD_SUBDIRECTORY(images)
ADD_SUBDIRECTORY(json)
ADD_SUBDIRECTORY(mpxlib)
ADD_SUBDIRECTORY(python)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(themes)

IF(ENABLE_HAL)
  ADD_SUBDIRECTORY(libhal++)
ENDIF()

# Installation

FILE(GLOB DBUS_SERVICE_FILES "${PROJECT_BINARY_DIR}/*.service")

INSTALL(FILES ${PROJECT_BINARY_DIR}/mpx.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
INSTALL(FILES ${PROJECT_BINARY_DIR}/mpx.1  DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man1)
INSTALL(FILES ${DBUS_SERVICE_FILES} DESTINATION ${DBUS_SERVICES_DIR})

# Uninstallation
# Script copied from CMake FAQ

CONFIGURE_FILE(
  "${PROJECT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${PROJECT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY
)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${PROJECT_BINARY_DIR}/cmake_uninstall.cmake"
)

# Source packaging

SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${PACKAGE}-${PACKAGE_VERSION}")
SET(CPACK_SOURCE_GENERATOR TGZ TBZ2)

SET(CPACK_SOURCE_IGNORE_FILES
  /\\\\.git/
  /\\\\.gitignore$
  /_CPack_Packages/
  /CMakeCache.txt$
  CMakeFiles/
  /CPackConfig.cmake$
  /CPackSourceConfig.cmake$
  cmake_install.cmake$
  cmake_uninstall.cmake$
  Makefile$
  /install_manifest.txt$
  \\\\.service$
  \\\\.desktop$
  \\\\.pc$
  \\\\.o$
  \\\\.a$
  \\\\.so$
  \\\\.so\\\\..*$
  \\\\.mo$
  \\\\.tar\\\\.gz$
  \\\\.tar\\\\.bz2$
)

INCLUDE(CPack)
