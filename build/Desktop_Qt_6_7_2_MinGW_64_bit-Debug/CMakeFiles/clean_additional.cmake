# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\sql_minus_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\sql_minus_autogen.dir\\ParseCache.txt"
  "sql_minus_autogen"
  )
endif()
