# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\test_of_sqlminus_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\test_of_sqlminus_autogen.dir\\ParseCache.txt"
  "test_of_sqlminus_autogen"
  )
endif()
