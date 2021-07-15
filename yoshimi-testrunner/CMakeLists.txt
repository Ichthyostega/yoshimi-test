#
# Yoshimi-testrunner  -  perform automated acceptance tests for the Yoshimi soft synthesizer
#
# Copyright 2021, Hermann Vosseler <Ichthyostega@web.de>
#
# This program is free software: you can redistribute and modify it
# under the terms of the GNU General Public License version GPL v3+
#
cmake_minimum_required(VERSION 3.12)

project(
    YoshimiTest
    VERSION 0.1
    LANGUAGES CXX
)


# dependencies via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSOX REQUIRED IMPORTED_TARGET sox>=14)


# build complete source tree (detect changes on rebuild)
file(GLOB_RECURSE testrunner_sources CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE testrunner_headers CONFIGURE_DEPENDS "src/*.hpp")

add_executable(testrunner ${testrunner_sources} ${testrunner_headers})
target_compile_features(testrunner PUBLIC cxx_std_17)
target_link_libraries(testrunner PRIVATE PkgConfig::LIBSOX)
target_link_libraries(testrunner PRIVATE stdc++fs)
