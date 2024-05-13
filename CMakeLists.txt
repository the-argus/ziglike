cmake_minimum_required(VERSION 3.8...3.26)

project(ziglike CXX)

set(headers
    ziglike.h
    ziglike/anystatus.h
    ziglike/defer.h
    ziglike/enumerate.h
    ziglike/factory.h
    ziglike/opt.h
    ziglike/res.h
    ziglike/slice.h
    ziglike/status.h
    ziglike/stdmem.h
    ziglike/try.h
    ziglike/zigstdint.h
    ziglike/detail/abort.h
    ziglike/detail/is_container.h
    ziglike/detail/isinstance.h
)

# TODO: have cmake and pkg-config config files so that it makes sense to install ziglike
option(ZIGLIKE_INSTALL "Generate the install target." ON)
option(ZIGLIKE_SYSTEM_HEADERS "Expose headers with marking them as system." OFF)

set(ZIGLIKE_INC_DIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE STRING
    "Installation directory for include files, a relative path that "
    "will be joined with ${CMAKE_INSTALL_PREFIX} or an absolute path.")

set(ZIGLIKE_SYSTEM_HEADERS_ATTRIBUTE "")
if (ZIGLIKE_SYSTEM_HEADERS)
    set(ZIGLIKE_SYSTEM_HEADERS_ATTRIBUTE SYSTEM)
endif()

add_library(ziglike-header-only INTERFACE)
add_library(ziglike::ziglike-header-only ALIAS ziglike-header-only)
target_compile_definitions(ziglike-header-only INTERFACE)
target_compile_features(ziglike-header-only INTERFACE cxx_std_17)

target_include_directories(ziglike-header-only ${ZIGLIKE_SYSTEM_HEADERS_ATTRIBUTE} INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${ZIGLIKE_INC_DIR}>)

if (ZIGLIKE_INSTALL)
  set(INSTALL_TARGETS ziglike-header-only)

  set_verbose(FMT_LIB_DIR ${CMAKE_INSTALL_LIBDIR} CACHE STRING
              "Installation directory for libraries, a relative path that "
              "will be joined to ${CMAKE_INSTALL_PREFIX} or an absolute path.")

  install(TARGETS ${INSTALL_TARGETS}
          LIBRARY DESTINATION ${FMT_LIB_DIR}
          ARCHIVE DESTINATION ${FMT_LIB_DIR}
          PUBLIC_HEADER DESTINATION "${ZIGLIKE_INC_DIR}/fmt"
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
