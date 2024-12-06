# Evolution example-module build script

cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.6)

project(example-modules C)

#
# Base bits
#
set(GETTEXT_PACKAGE "example-module")
set(RELEASE_NAME "Evolution example module.")
set(VERSION "0.0.1")
set(VERSION_INFO "Release")
set(LANGUAGE_SUPPORT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/share/locale)

add_definitions(-DGETTEXT_PACKAGE=\"${GETTEXT_PACKAGE}\")

# Filenames for tarball
set(ARCHIVE_BASE_NAME ${CMAKE_PROJECT_NAME}-${VERSION})
set(ARCHIVE_FULL_NAME ${ARCHIVE_BASE_NAME}.tar.xz)

set(REQUIRE_EVOLUTION_VERSION 3.36.0)

find_package(PkgConfig REQUIRED)

function(pkg_check_variable _output_name _pkg _name)
    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=${_name} ${_pkg}
                    OUTPUT_VARIABLE _pkg_result
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    set("${_output_name}" "${_pkg_result}" CACHE STRING "pkg-config variable ${_name} of ${_pkg}")
endfunction()

pkg_check_modules(EVOLUTION_SHELL REQUIRED evolution-shell-3.0>=${REQUIRE_EVOLUTION_VERSION})
pkg_check_modules(EVOLUTION_CALENDAR REQUIRED evolution-calendar-3.0>=${REQUIRE_EVOLUTION_VERSION})
pkg_check_modules(EVOLUTION_MAIL REQUIRED evolution-mail-3.0>=${REQUIRE_EVOLUTION_VERSION})
pkg_check_modules(LIBECAL REQUIRED libecal-2.0>=${REQUIRE_EVOLUTION_VERSION})

pkg_check_variable(EVOLUTION_MODULE_DIR evolution-shell-3.0 moduledir)

option(FORCE_INSTALL_PREFIX "Whether to force install evolution-data-server and evolution files into the install prefix" OFF)
if(FORCE_INSTALL_PREFIX)
	pkg_check_variable(evo_prefix evolution-shell-3.0 prefix)
	string(REGEX REPLACE "^${evo_prefix}" "${CMAKE_INSTALL_PREFIX}" EVOLUTION_MODULE_DIR "${EVOLUTION_MODULE_DIR}")
endif(FORCE_INSTALL_PREFIX)

include(CheckCCompilerFlag)
check_c_compiler_flag(-Wno-deprecated-declarations HAVE_NO_DEPRECATED_DECLARATIONS)

if(HAVE_NO_DEPRECATED_DECLARATIONS)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")
endif()

# uninstall target
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)

add_custom_target(uninstall
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# Dist
# This generates the dist tarball from a git clone
add_custom_target(
	dist
	COMMAND git archive --prefix=${ARCHIVE_BASE_NAME}/ HEAD | xz -z > ${CMAKE_BINARY_DIR}/${ARCHIVE_FULL_NAME}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_subdirectory(src)
