cmake_minimum_required(VERSION 3.16)

#Git component#
set (EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_LIST_DIR}/micro_ros_espidf_component/components)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

#Con esto se acepta cpp#
project(Prueba_ESP32 LANGUAGES C CXX)
