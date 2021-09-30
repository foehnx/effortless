cmake_minimum_required(VERSION 2.8.12...3.13)

project(catch2-external)

include(ExternalProject)
ExternalProject_Add(catch2
  GIT_REPOSITORY    https://github.com/catchorg/Catch2.git
  GIT_TAG           v2.13.4
  GIT_SHALLOW       True
  GIT_PROGRESS      True
  GIT_CONFIG        "advice.detachedHead=false"
  SOURCE_DIR        "${PROJECT_SOURCE_DIR}/externals/catch2"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
  UPDATE_DISCONNECTED ON
)
