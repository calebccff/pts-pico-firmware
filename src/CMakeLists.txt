add_executable(${CMAKE_PROJECT_NAME} main.c usb-descriptors.c)
target_link_libraries(${CMAKE_PROJECT_NAME} pico_stdlib tinyusb_device pico_unique_id)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ./ ${pico_sdk_SOURCE_DIR}/lib/tinyusb/src)
pico_add_extra_outputs(${CMAKE_PROJECT_NAME})
#pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)
