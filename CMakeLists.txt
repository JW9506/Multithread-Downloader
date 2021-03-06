cmake_minimum_required(VERSION 3.0.0)
project(main VERSION 0.1.0)

set(CMAKE_C_STANDARD 11)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(PkgConfig REQUIRED)

if (WIN32)
  set(DEPS gtk+-3.0 libzstd)
elseif (APPLE)
  set(DEPS gtk+-3.0 glib-2.0)
else()
  set(DEPS gtk+-3.0)
endif()

foreach (dep ${DEPS})
  PKG_CHECK_MODULES(${dep} REQUIRED ${dep})
  include_directories(${${dep}_INCLUDE_DIRS})
  link_directories(${${dep}_LIBRARY_DIRS})
  add_definitions(${${dep}_CFLAGS_DIRS})
endforeach()

include_directories("include")

set(CMAKE_C_FLAGS_RELEASE -mwindows)

set(CMAKE_INSTALL_PREFIX install)

set(CMAKE_INSTALL_RPATH \$ORIGIN/../lib)

file(GLOB_RECURSE files "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_executable(main main.c ${files})
target_link_libraries(main ${CONAN_LIBS})

foreach (dep ${DEPS})
  target_link_libraries(main ${${dep}_LIBRARIES})
endforeach()

install(TARGETS main RUNTIME DESTINATION bin)
