# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\Shapez_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\Shapez_autogen.dir\\ParseCache.txt"
  "Shapez_autogen"
  )
endif()
