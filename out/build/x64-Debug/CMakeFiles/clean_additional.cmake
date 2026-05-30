# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "AlwaysCDRipper_autogen"
  "CMakeFiles\\AlwaysCDRipper_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\AlwaysCDRipper_autogen.dir\\ParseCache.txt"
  )
endif()
