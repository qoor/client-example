cmake_minimum_required(VERSION 3.0)

project(client-example LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(tinyxml tinyxml2 REQUIRED)
pkg_check_modules(openssl openssl REQUIRED)

set(SOURCES
  client/client.cc)

add_executable(client ${SOURCES})
set_target_properties(client PROPERTIES CXX_STANDARD 14 C_STANDARD 90)
target_compile_options(client PRIVATE -W -Wall -Wextra -Werror)
target_link_libraries(client ${tinyxml_LIBRARIES} ${openssl_LIBRARIES})
target_include_directories(client PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/common
  ${tinyxml_INCLUDE_DIRS} ${openssl_INCLUDE_DIRS})
