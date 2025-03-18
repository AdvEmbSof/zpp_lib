zephyr_library()
FILE(GLOB app_sources zpp_rtos/*.cpp zpp_drivers/*.cpp)

zephyr_library_sources(${app_sources})
