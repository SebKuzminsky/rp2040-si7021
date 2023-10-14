set(LIBRARY_NAME rp2040_si7021)

add_library(
    ${LIBRARY_NAME}
    STATIC
    si7021.cpp
)

target_compile_options(
    ${LIBRARY_NAME}
    PRIVATE
    "-Wall"
)

target_link_libraries(
    ${LIBRARY_NAME}
    pico_stdlib
    hardware_i2c
)

target_include_directories(
    ${LIBRARY_NAME}
    PUBLIC
    "./include"
)
