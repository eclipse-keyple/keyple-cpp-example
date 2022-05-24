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


SET(CALYPSONET_CARD_DIR    "../../calypsonet-terminal-card-cpp-api")
SET(CALYPSONET_READER_DIR  "../../calypsonet-terminal-reader-cpp-api")
SET(KEYPLE_CARD_DIR        "../../keyple-card-generic-cpp-lib")
SET(KEYPLE_COMMON_DIR      "../../keyple-common-cpp-api")
SET(KEYPLE_PLUGIN_DIR      "../../keyple-plugin-cpp-api")
SET(KEYPLE_PCSC_DIR        "../../keyple-plugin-pcsc-cpp-lib")
SET(KEYPLE_STUB_DIR        "../../keyple-plugin-stub-cpp-lib")
SET(KEYPLE_RESOURCE_DIR    "../../keyple-service-resource-cpp-lib")
SET(KEYPLE_SERVICE_DIR     "../../keyple-service-cpp-lib")
SET(KEYPLE_UTIL_DIR        "../../keyple-util-cpp-lib")

SET(KEYPLE_STUB_LIB        "keyplepluginstubcpplib")
SET(KEYPLE_PCSC_LIB        "keyplepluginpcsccpplib")
SET(KEYPLE_CARD_LIB        "keyplecardgenericcpplib")
SET(KEYPLE_SERVICE_LIB     "keypleservicecpplib")
SET(KEYPLE_RESOURCE_LIB    "keypleserviceresourcecpplib")
SET(KEYPLE_UTIL_LIB        "keypleutilcpplib")

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common

    ${CALYPSONET_CARD_DIR}/src/main
    ${CALYPSONET_CARD_DIR}/src/main/spi

    ${CALYPSONET_READER_DIR}/src/main
    ${CALYPSONET_READER_DIR}/src/main/selection
    ${CALYPSONET_READER_DIR}/src/main/selection/spi
    ${CALYPSONET_READER_DIR}/src/main/spi

    ${KEYPLE_COMMON_DIR}/src/main

    ${KEYPLE_CARD_DIR}/src/main

    ${KEYPLE_PCSC_DIR}/src/main

    ${KEYPLE_PLUGIN_DIR}/src/main
    ${KEYPLE_PLUGIN_DIR}/src/main/spi
    ${KEYPLE_PLUGIN_DIR}/src/main/spi/reader
    ${KEYPLE_PLUGIN_DIR}/src/main/spi/reader/observable/
    ${KEYPLE_PLUGIN_DIR}/src/main/spi/reader/observable/state/insertion
    ${KEYPLE_PLUGIN_DIR}/src/main/spi/reader/observable/state/processing
    ${KEYPLE_PLUGIN_DIR}/src/main/spi/reader/observable/state/removal

    ${KEYPLE_RESOURCE_DIR}/src/main
    ${KEYPLE_RESOURCE_DIR}/src/main/spi

    ${KEYPLE_SERVICE_DIR}/src/main
    ${KEYPLE_SERVICE_DIR}/src/main/cpp
    ${KEYPLE_SERVICE_DIR}/src/main/spi

    ${KEYPLE_UTIL_DIR}/src/main
    ${KEYPLE_UTIL_DIR}/src/main/cpp
    ${KEYPLE_UTIL_DIR}/src/main/cpp/exception
    ${KEYPLE_UTIL_DIR}/src/main/protocol

    ${KEYPLE_STUB_DIR}/src/main
)

SET(USECASE1 UseCase1_CardResourceService)
ADD_EXECUTABLE(${USECASE1}
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE1}/Main_CardResourceService_Stub.cpp)
TARGET_LINK_LIBRARIES(${USECASE1}

                      ${KEYPLE_CARD_LIB}
                      ${KEYPLE_STUB_LIB}
                      ${KEYPLE_SERVICE_LIB}
                      ${KEYPLE_RESOURCE_LIB}
                      ${KEYPLE_UTIL_LIB}
)

IF(APPLE)
    TARGET_LINK_LIBRARIES(${USECASE1} pthread)
ELSEIF(UNIX)
    TARGET_LINK_LIBRARIES(${USECASE1} pthread)
ENDIF(APPLE)
