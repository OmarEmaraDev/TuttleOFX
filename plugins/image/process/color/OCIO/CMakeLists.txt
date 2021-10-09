# Macros used to create an openfx plugin with tuttle
include(TuttleMacros)

# Declare the plugin
tuttle_ofx_plugin_target(OCIO)

find_package(OpenColorIO)
target_link_libraries(OCIO OpenColorIO::OpenColorIO)
