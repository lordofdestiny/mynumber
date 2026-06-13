cmake_minimum_required(VERSION 3.20)

get_filename_component(MYNUMBER_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)
if(CMAKE_SOURCE_DIR STREQUAL MYNUMBER_CMAKE_DIR)
  set(MYNUMBER_ROOT "${MYNUMBER_CMAKE_DIR}")
else()
  set(MYNUMBER_ROOT "${CMAKE_SOURCE_DIR}")
endif()

file(READ "${MYNUMBER_ROOT}/packaging/project.json" MYNUMBER_PROJECT_JSON)
string(JSON MYNUMBER_PROJECT_VERSION GET ${MYNUMBER_PROJECT_JSON} version)

project(mynumber VERSION ${MYNUMBER_PROJECT_VERSION} LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
endif()

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(MYNUMBER_IMPL_SOURCES
  ${MYNUMBER_ROOT}/src/impl/Combination.cpp
  ${MYNUMBER_ROOT}/src/impl/Operator.cpp
  ${MYNUMBER_ROOT}/src/impl/StateValue.cpp
)

add_library(mynumber_static STATIC ${MYNUMBER_IMPL_SOURCES})
add_library(mynumber_shared SHARED ${MYNUMBER_IMPL_SOURCES})

set(MYNUMBER_COMPILE_OPTIONS -Wall -Wextra -Werror -O3)

target_compile_options(mynumber_static PRIVATE ${MYNUMBER_COMPILE_OPTIONS})
target_compile_options(mynumber_shared PRIVATE ${MYNUMBER_COMPILE_OPTIONS})
target_compile_definitions(mynumber_shared PRIVATE MYNUMBER_SHARED_BUILD)

target_include_directories(mynumber_static PUBLIC
  $<BUILD_INTERFACE:${MYNUMBER_ROOT}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(mynumber_shared PUBLIC
  $<BUILD_INTERFACE:${MYNUMBER_ROOT}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(mynumber_static mynumber_shared PROPERTIES
  OUTPUT_NAME mynumber
)

set_target_properties(mynumber_shared PROPERTIES
  INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
  INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
  BUILD_WITH_INSTALL_RPATH FALSE
)

add_library(mynumber::mynumber_static ALIAS mynumber_static)
add_library(mynumber::mynumber_shared ALIAS mynumber_shared)

option(MYNUMBER_BUILD_CLI "Build CLI executable" ON)

set(MYNUMBER_INSTALL_TARGETS mynumber_static mynumber_shared)

if(MYNUMBER_BUILD_CLI)
  add_executable(mynumber_cli ${MYNUMBER_ROOT}/src/main.cpp)
  target_link_libraries(mynumber_cli PRIVATE mynumber_static)
  target_compile_options(mynumber_cli PRIVATE ${MYNUMBER_COMPILE_OPTIONS})
  set_target_properties(mynumber_cli PROPERTIES OUTPUT_NAME mynumber)
  list(APPEND MYNUMBER_INSTALL_TARGETS mynumber_cli)
endif()

install(TARGETS ${MYNUMBER_INSTALL_TARGETS}
  EXPORT mynumberTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${MYNUMBER_ROOT}/include/mynumber.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY ${MYNUMBER_ROOT}/include/impl ${MYNUMBER_ROOT}/include/polyfill DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT mynumberTargets
  FILE mynumberTargets.cmake
  NAMESPACE mynumber::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mynumber
)

configure_package_config_file(
  "${MYNUMBER_CMAKE_DIR}/cmake/mynumberConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/mynumberConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mynumber
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/mynumberConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/mynumberConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/mynumberConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mynumber
)

configure_file(
  "${MYNUMBER_CMAKE_DIR}/cmake/mynumber.pc.in"
  "${CMAKE_CURRENT_BINARY_DIR}/mynumber.pc"
  @ONLY
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mynumber.pc"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

set(CPACK_PACKAGE_NAME mynumber)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Number combination puzzle solver C++ library")
set(CPACK_PACKAGE_VENDOR "lordofdestiny")
set(CPACK_GENERATOR "TGZ;ZIP")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)

include(CPack)
