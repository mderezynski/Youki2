CONFIGURE_FILE(__init__.py.in ${CMAKE_CURRENT_BINARY_DIR}/__init__.py)

INSTALL(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/__init__.py
        mpxdj.glade
        mpxdj-icon.png
        mpxdj.png
        mpxdj.youki-plugin
  DESTINATION ${MPX_PLUGIN_DIR}/plugins-python/mpxdj
)
