cmake_minimum_required(VERSION 3.10)


include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

# Set the project name and version
project(radio_mqtt_bridge VERSION 1.0 DESCRIPTION "Radio to MQTT Bridge")

# Specify the C standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)
set(PICO_DEOPTIMIZED_DEBUG 1)
set(PICO_BOARD pico_w)

# # initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# add the sources 
file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(radio_mqtt_bridge
    ${SOURCES}
)

target_include_directories(radio_mqtt_bridge PRIVATE src/include)

add_subdirectory(lib/rp-pico-rfm69-driver)
add_subdirectory(lib/nanopb)


target_link_libraries(
    radio_mqtt_bridge 
    pico_stdlib 
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
    hardware_spi 
    hardware_sleep
    rfm69 
    protobuf-nanopb-static 
)

# enable usb output, disable uart output
pico_enable_stdio_usb(radio_mqtt_bridge 0)
pico_enable_stdio_uart(radio_mqtt_bridge 1)

pico_add_extra_outputs(radio_mqtt_bridge)
