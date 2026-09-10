# *****************************************************************************
# Copyright (c) 2025 Calypso Networks Association https://calypsonet.org/     *
#                                                                             *
# See the NOTICE file(s) distributed with this work for additional            *
# information regarding copyright ownership.                                  *
#                                                                             *
# This program and the accompanying materials are made available under the    *
# terms of the Eclipse                                                        *
#                                                                             *
# Public License 2.0 which is available at                                    *
# http://www.eclipse.org/legal/epl-2.0                                        *
#                                                                             *
# SPDX-License-Identifier: EPL-2.0                                            *
# *****************************************************************************/

CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

PROJECT(KeypleCppExample
        VERSION 2024.04.18
        LANGUAGES C CXX)

SET(CMAKE_PROJECT_VERSION_MAJOR "2024")
SET(CMAKE_PROJECT_VERSION_MINOR "04")
SET(CMAKE_PROJECT_VERSION_PATCH "18")

SET(CMAKE_PROJECT_VERSION "${CMAKE_PROJECT_VERSION_MAJOR}.
                           ${CMAKE_PROJECT_VERSION_MINOR}.
                           ${CMAKE_PROJECT_VERSION_PATCH}")

SET(PACKAGE_NAME "KeypleCppExample")
SET(PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
SET(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")

SET(CMAKE_LEGACY_CYGWIN_WIN32 0)
SET(CMAKE_CXX_STANDARD 11)

# Compilers
SET(CMAKE_C_COMPILER_WORKS 1)
SET(CMAKE_CXX_COMPILER_WORKS 1)

IF(NOT TOOLCHAIN_FILE)
    MESSAGE(FATAL_ERROR "Toolchain file not specified")
ENDIF()

# Set common output directory
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include deps
INCLUDE("CMakeLists.txt.keyple-card-calypso")
INCLUDE("CMakeLists.txt.keyple-card-generic")
INCLUDE("CMakeLists.txt.keyple-common")
INCLUDE("CMakeLists.txt.keyple-plugin")
INCLUDE("CMakeLists.txt.keyple-plugin-stub")
INCLUDE("CMakeLists.txt.keyple-plugin-pcsc")
INCLUDE("CMakeLists.txt.keyple-service")
INCLUDE("CMakeLists.txt.keyple-util")
INCLUDE("CMakeLists.txt.keypop-card")
INCLUDE("CMakeLists.txt.keypop-reader")

# Add projects
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/Example_Card_Calypso)
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/Example_Plugin_PCSC)
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/Example_Service)
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/Example_Service_Resource)
