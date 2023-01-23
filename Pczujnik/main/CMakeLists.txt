# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS "main.c" "include/gaz.c" "include/ble.c" "include/ble-write.c" "include/bmp280_i2c_hal.c" "include/bmp280_i2c.c" "include/ble.c" "include/read_sensor.c" "include/mqtt-pub.c"      # list the source files of this component
    INCLUDE_DIRS        # optional, add here public include directories
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)
