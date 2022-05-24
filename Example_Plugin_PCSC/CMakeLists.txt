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
SET(KEYPLE_RESOURCE_DIR    "../../keyple-service-resource-cpp-lib")
SET(KEYPLE_SERVICE_DIR     "../../keyple-service-cpp-lib")
SET(KEYPLE_UTIL_DIR        "../../keyple-util-cpp-lib")

SET(KEYPLE_CARD_LIB        "keyplecardgenericcpplib")
SET(KEYPLE_UTIL_LIB        "keypleutilcpplib")
SET(KEYPLE_SERVICE_LIB     "keypleservicecpplib")
SET(KEYPLE_PCSC_LIB        "keyplepluginpcsccpplib")

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
)

SET(USECASE1 UseCase1_ReaderTypeAutoIdentification)
ADD_EXECUTABLE(${USECASE1}
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE1}/Main_ReaderTypeAutoIdentification_Pcsc.cpp)
TARGET_LINK_LIBRARIES(${USECASE1} ${KEYPLE_CARD_LIB} ${KEYPLE_PCSC_LIB} ${KEYPLE_SERVICE_LIB} ${KEYPLE_UTIL_LIB})

SET(USECASE2 UseCase2_ExplicitReaderType)
ADD_EXECUTABLE(${USECASE2}
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE2}/Main_ExplicitReaderType_Pcsc.cpp)
TARGET_LINK_LIBRARIES(${USECASE2} ${KEYPLE_CARD_LIB} ${KEYPLE_PCSC_LIB} ${KEYPLE_SERVICE_LIB} ${KEYPLE_UTIL_LIB})

SET(USECASE3 UseCase3_ChangeProtocolRules)
ADD_EXECUTABLE(${USECASE3}
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE3}/Main_ChangeProtocolRules_Pcsc.cpp)
TARGET_LINK_LIBRARIES(${USECASE3} ${KEYPLE_CARD_LIB} ${KEYPLE_PCSC_LIB} ${KEYPLE_SERVICE_LIB} ${KEYPLE_UTIL_LIB})
