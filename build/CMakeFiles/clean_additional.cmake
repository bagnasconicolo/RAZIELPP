# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/RazielNDVIpp_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/RazielNDVIpp_autogen.dir/ParseCache.txt"
  "RazielNDVIpp_autogen"
  )
endif()
