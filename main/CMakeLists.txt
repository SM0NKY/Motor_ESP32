idf_component_register(
  SRCS "main.cpp" "GPS.cpp" "micro_ros_serial_transport.cpp"
  INCLUDE_DIRS "."
  REQUIRES micro_ros_espidf_component
)