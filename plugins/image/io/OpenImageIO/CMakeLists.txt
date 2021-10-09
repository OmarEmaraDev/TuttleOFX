# Macros used to create an openfx plugin with tuttle
include(TuttleMacros)

tuttle_ofx_plugin_target(OpenImageIO)

# Add external libraries
tuttle_ofx_plugin_add_library(OpenImageIO sequenceParser)

# FIXME : as Terry needs to load half.h
find_package(OpenEXR)
target_link_libraries(OpenImageIO OpenEXR::OpenEXR)

find_package(Imath)
target_link_libraries(OpenImageIO Imath::Imath)

find_package(OpenImageIO)
target_link_libraries(OpenImageIO OpenImageIO::OpenImageIO)
