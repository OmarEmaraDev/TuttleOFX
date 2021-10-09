# Macros used to create an openfx plugin with tuttle
include(TuttleMacros)

# Declare the plugin
tuttle_ofx_plugin_target(CTL)
tuttle_ofx_plugin_add_library(CTL CTL)

find_package(OpenEXR)
target_link_libraries(CTL OpenEXR::OpenEXR)

find_package(Imath)
target_link_libraries(CTL Imath::Imath)
