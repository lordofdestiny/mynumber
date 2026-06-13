cmake_minimum_required(VERSION 3.20)

project(mynumber VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(MYNUMBER_IMPL_SOURCES
  src/impl/Combination.cpp
  src/impl/Operator.cpp
  src/impl/StateValue.cpp
)

add_library(mynumber_static STATIC ${MYNUMBER_IMPL_SOURCES})
add_library(mynumber_shared SHARED ${MYNUMBER_IMPL_SOURCES})

set(MYNUMBER_COMPILE_OPTIONS -Wall -Wextra -Werror -O3)

target_compile_options(mynumber_static PRIVATE ${MYNUMBER_COMPILE_OPTIONS})
target_compile_options(mynumber_shared PRIVATE ${MYNUMBER_COMPILE_OPTIONS})
target_compile_definitions(mynumber_shared PRIVATE MYNUMBER_SHARED_BUILD)

target_include_directories(mynumber_static PUBLIC include)
target_include_directories(mynumber_shared PUBLIC include)

set_target_properties(mynumber_static PROPERTIES OUTPUT_NAME mynumber)
set_target_properties(mynumber_shared PROPERTIES OUTPUT_NAME mynumber)

option(MYNUMBER_BUILD_CLI "Build CLI executable" ON)

set(MYNUMBER_INSTALL_TARGETS mynumber_static mynumber_shared)

if(MYNUMBER_BUILD_CLI)
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    message(FATAL_ERROR "MYNUMBER_BUILD_CLI=ON but src/main.cpp is missing")
  endif()
  add_executable(mynumber_cli src/main.cpp)
  target_link_libraries(mynumber_cli PRIVATE mynumber_static)
  target_compile_options(mynumber_cli PRIVATE ${MYNUMBER_COMPILE_OPTIONS})
  set_target_properties(mynumber_cli PROPERTIES OUTPUT_NAME mynumber)
  list(APPEND MYNUMBER_INSTALL_TARGETS mynumber_cli)
endif()

install(TARGETS ${MYNUMBER_INSTALL_TARGETS}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(FILES include/mynumber.hpp DESTINATION include)
install(DIRECTORY include/impl include/polyfill DESTINATION include)

include(CPack)
set(CPACK_PACKAGE_NAME mynumber)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Number combination puzzle solver C++ library")
set(CPACK_PACKAGE_VENDOR "lordofdestiny")
set(CPACK_GENERATOR TGZ ZIP)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
