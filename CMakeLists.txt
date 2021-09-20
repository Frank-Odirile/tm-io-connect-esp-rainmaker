cmake_minimum_required(VERSION 3.16.0)

list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_LIST_DIR}/esp-rainmaker/components ${CMAKE_CURRENT_LIST_DIR}/esp-rainmaker/components/esp-insights/components)

# Project
set(PROJECT_VER "v1.0.1_210921")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(tm_io_connect_esp_rainmaker)
