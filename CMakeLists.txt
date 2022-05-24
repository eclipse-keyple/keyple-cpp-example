# *************************************************************************************************
# Copyright (c) 2021 Calypso Networks Association https://calypsonet.org/                         *
#                                                                                                 *
# See the NOTICE file(s) distributed with this work for additional information regarding          *
# copyright ownership.                                                                            *
#                                                                                                 *
# This program and the accompanying materials are made available under the terms of the Eclipse   *
# Public License 2.0 which is available at http://www.eclipse.org/legal/epl-2.0                   *
#                                                                                                 *
# SPDX-License-Identifier: EPL-2.0                                                                *
# *************************************************************************************************/

PROJECT(KeypleCppExample C CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(CMAKE_PROJECT_VERSION_MAJOR "2")
SET(CMAKE_PROJECT_VERSION_MINOR "0")
SET(CMAKE_PROJECT_VERSION_PATCH "0")

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

# Add projects
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/Example_Card_Calypso)
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/Example_Plugin_PCSC)
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/Example_Service)
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/Example_Service_Resource)
