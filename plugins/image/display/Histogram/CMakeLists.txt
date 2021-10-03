# Macros used to create an openfx plugin with tuttle
include(TuttleMacros)

# Declare the plugin
tuttle_ofx_plugin_target(Histogram)

add_definitions(-DWITH_BOOST_ATOMIC)
