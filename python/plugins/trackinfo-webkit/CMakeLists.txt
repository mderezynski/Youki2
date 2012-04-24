CONFIGURE_FILE(__init__.py.in ${CMAKE_CURRENT_BINARY_DIR}/__init__.py)

INSTALL(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/__init__.py
        trackinfo-webkit.glade
        trackinfo-webkit.youki-plugin
  DESTINATION ${MPX_PLUGIN_DIR}/plugins-python/trackinfo-webkit
)
