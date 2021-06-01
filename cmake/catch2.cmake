# Download and unpack Catch2 at configure time
message(STATUS "Getting Catch2...")

configure_file(
  cmake/catch2_download.cmake
  ${PROJECT_SOURCE_DIR}/externals/catch2-download/CMakeLists.txt)

execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/externals/catch2-download
  OUTPUT_QUIET)
if(result)
  message(FATAL_ERROR "Download of Catch2 failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/externals/catch2-download
  OUTPUT_QUIET)
if(result)
  message(FATAL_ERROR "Build step for Catch2 failed: ${result}")
endif()

message(STATUS "Catch2 downloaded!")

add_subdirectory(${PROJECT_SOURCE_DIR}/externals/catch2)
# set(CATCH2_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/externals/catch2/include)

