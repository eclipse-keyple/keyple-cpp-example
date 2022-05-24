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


SET(CALYPSONET_CALYPSO_DIR "../../calypsonet-terminal-calypso-cpp-api")
SET(CALYPSONET_CARD_DIR    "../../calypsonet-terminal-card-cpp-api")
SET(CALYPSONET_READER_DIR  "../../calypsonet-terminal-reader-cpp-api")
SET(KEYPLE_CALYPSO_DIR     "../../keyple-card-calypso-cpp-lib")
SET(KEYPLE_COMMON_DIR      "../../keyple-common-cpp-api")
SET(KEYPLE_PLUGIN_DIR      "../../keyple-plugin-cpp-api")
SET(KEYPLE_PCSC_DIR        "../../keyple-plugin-pcsc-cpp-lib")
SET(KEYPLE_RESOURCE_DIR    "../../keyple-service-resource-cpp-lib")
SET(KEYPLE_SERVICE_DIR     "../../keyple-service-cpp-lib")
SET(KEYPLE_STUB_DIR        "../../keyple-plugin-stub-cpp-lib")
SET(KEYPLE_UTIL_DIR        "../../keyple-util-cpp-lib")

SET(KEYPLE_CALYPSO_LIB     "keyplecardcalypsocpplib")
SET(KEYPLE_PCSC_LIB        "keyplepluginpcsccpplib")
SET(KEYPLE_RESOURCE_LIB    "keypleserviceresourcecpplib")
SET(KEYPLE_SERVICE_LIB     "keypleservicecpplib")
SET(KEYPLE_STUB_LIB        "keyplepluginstubcpplib")
SET(KEYPLE_UTIL_LIB        "keypleutilcpplib")

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/spi

    ${CALYPSONET_CALYPSO_DIR}/src/main
    ${CALYPSONET_CALYPSO_DIR}/src/main/card
    ${CALYPSONET_CALYPSO_DIR}/src/main/sam
    ${CALYPSONET_CALYPSO_DIR}/src/main/transaction

    ${CALYPSONET_CARD_DIR}/src/main
    ${CALYPSONET_CARD_DIR}/src/main/spi

    ${CALYPSONET_READER_DIR}/src/main
    ${CALYPSONET_READER_DIR}/src/main/selection
    ${CALYPSONET_READER_DIR}/src/main/selection/spi
    ${CALYPSONET_READER_DIR}/src/main/spi

    ${KEYPLE_CALYPSO_DIR}/src/main

    ${KEYPLE_CARD_DIR}/src/main

    ${KEYPLE_COMMON_DIR}/src/main

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

    ${KEYPLE_STUB_DIR}/src/main

    ${KEYPLE_UTIL_DIR}/src/main
    ${KEYPLE_UTIL_DIR}/src/main/cpp
    ${KEYPLE_UTIL_DIR}/src/main/cpp/exception
    ${KEYPLE_UTIL_DIR}/src/main/protocol
)

SET(USECASE1 UseCase1_ExplicitSelectionAid)
SET(USECASE1_STUB ${USECASE1}_Stub)
ADD_EXECUTABLE(${USECASE1_STUB}
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/StubSmartCardFactory.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE1}/Main_ExplicitSelectionAid_Stub.cpp)
TARGET_LINK_LIBRARIES(${USECASE1_STUB} ${KEYPLE_CARD_LIB} ${KEYPLE_STUB_LIB} ${KEYPLE_SERVICE_LIB} ${KEYPLE_UTIL_LIB} ${KEYPLE_CALYPSO_LIB} pthread)

SET(USECASE1_PCSC ${USECASE1}_Pcsc)
ADD_EXECUTABLE(${USECASE1_PCSC}
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE1}/Main_ExplicitSelectionAid_Pcsc.cpp)
TARGET_LINK_LIBRARIES(${USECASE1_PCSC} ${KEYPLE_CARD_LIB} ${KEYPLE_PCSC_LIB} ${KEYPLE_SERVICE_LIB} ${KEYPLE_UTIL_LIB} ${KEYPLE_CALYPSO_LIB} ${KEYPLE_RESOURCE_LIB} pthread)

SET(USECASE2 UseCase2_ScheduledSelection)
SET(USECASE2_STUB ${USECASE2}_Stub)
ADD_EXECUTABLE(${USECASE2_STUB}
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/StubSmartCardFactory.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE2}/CardReaderObserver.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE2}/Main_ScheduledSelection_Stub.cpp)
TARGET_LINK_LIBRARIES(${USECASE2_STUB} ${KEYPLE_CARD_LIB} ${KEYPLE_STUB_LIB} ${KEYPLE_SERVICE_LIB} ${KEYPLE_UTIL_LIB} ${KEYPLE_CALYPSO_LIB} ${KEYPLE_RESOURCE_LIB} pthread)

SET(USECASE2_PCSC ${USECASE2}_Pcsc)
ADD_EXECUTABLE(${USECASE2_PCSC}
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE2}/CardReaderObserver.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE2}/Main_ScheduledSelection_Pcsc.cpp)
TARGET_LINK_LIBRARIES(${USECASE2_PCSC} ${KEYPLE_CARD_LIB} ${KEYPLE_PCSC_LIB} ${KEYPLE_SERVICE_LIB} ${KEYPLE_UTIL_LIB} ${KEYPLE_CALYPSO_LIB} ${KEYPLE_RESOURCE_LIB} pthread)

SET(USECASE3 UseCase3_Rev1Selection)
SET(USECASE3_PCSC ${USECASE3}_Pcsc)
ADD_EXECUTABLE(${USECASE3_PCSC}
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE3}/Main_Rev1Selection_Pcsc.cpp)
TARGET_LINK_LIBRARIES(${USECASE3_PCSC} ${KEYPLE_CARD_LIB} ${KEYPLE_PCSC_LIB} ${KEYPLE_SERVICE_LIB} ${KEYPLE_UTIL_LIB} ${KEYPLE_CALYPSO_LIB} ${KEYPLE_RESOURCE_LIB} pthread)

SET(USECASE4 UseCase4_CardAuthentication)
SET(USECASE4_STUB ${USECASE4}_Stub)
ADD_EXECUTABLE(${USECASE4_STUB}
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/StubSmartCardFactory.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE4}/Main_CardAuthentication_Stub.cpp)
TARGET_LINK_LIBRARIES(${USECASE4_STUB} ${KEYPLE_CARD_LIB} ${KEYPLE_STUB_LIB} ${KEYPLE_PCSC_LIB} ${KEYPLE_SERVICE_LIB} ${KEYPLE_UTIL_LIB} ${KEYPLE_CALYPSO_LIB} ${KEYPLE_RESOURCE_LIB} pthread)

SET(USECASE4_PCSC ${USECASE4}_Pcsc)
ADD_EXECUTABLE(${USECASE4_PCSC}
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE4}/Main_CardAuthentication_Pcsc.cpp)
TARGET_LINK_LIBRARIES(${USECASE4_PCSC} ${KEYPLE_CARD_LIB} ${KEYPLE_PCSC_LIB} ${KEYPLE_SERVICE_LIB} ${KEYPLE_UTIL_LIB} ${KEYPLE_CALYPSO_LIB} ${KEYPLE_RESOURCE_LIB} pthread)

SET(USECASE5 UseCase5_MultipleSession)
SET(USECASE5_PCSC ${USECASE5}_Pcsc)
ADD_EXECUTABLE(${USECASE5_PCSC}
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE5}/Main_MultipleSession_Pcsc.cpp)
TARGET_LINK_LIBRARIES(${USECASE5_PCSC} ${KEYPLE_CARD_LIB} ${KEYPLE_PCSC_LIB} ${KEYPLE_SERVICE_LIB} ${KEYPLE_UTIL_LIB} ${KEYPLE_CALYPSO_LIB} ${KEYPLE_RESOURCE_LIB} pthread)

SET(USECASE6 UseCase6_VerifyPin)
SET(USECASE6_PCSC ${USECASE6}_Pcsc)
ADD_EXECUTABLE(${USECASE6_PCSC}
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE6}/Main_VerifyPin_Pcsc.cpp)
TARGET_LINK_LIBRARIES(${USECASE6_PCSC} ${KEYPLE_CARD_LIB} ${KEYPLE_PCSC_LIB} ${KEYPLE_SERVICE_LIB} ${KEYPLE_UTIL_LIB} ${KEYPLE_CALYPSO_LIB} ${KEYPLE_RESOURCE_LIB} pthread)

SET(USECASE7 UseCase7_StoredValue_SimpleReload)
SET(USECASE7_PCSC ${USECASE7}_Pcsc)
ADD_EXECUTABLE(${USECASE7_PCSC}
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE7}/Main_StoredValue_SimpleReload_Pcsc.cpp)
TARGET_LINK_LIBRARIES(${USECASE7_PCSC} ${KEYPLE_CARD_LIB} ${KEYPLE_PCSC_LIB} ${KEYPLE_SERVICE_LIB} ${KEYPLE_UTIL_LIB} ${KEYPLE_CALYPSO_LIB} ${KEYPLE_RESOURCE_LIB} pthread)

SET(USECASE8 UseCase8_StoredValue_DebitInSession)
SET(USECASE8_PCSC ${USECASE8}_Pcsc)
ADD_EXECUTABLE(${USECASE8_PCSC}
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE8}/Main_StoredValue_DebitInSession_Pcsc.cpp)
TARGET_LINK_LIBRARIES(${USECASE8_PCSC} ${KEYPLE_CARD_LIB} ${KEYPLE_PCSC_LIB} ${KEYPLE_SERVICE_LIB} ${KEYPLE_UTIL_LIB} ${KEYPLE_CALYPSO_LIB} ${KEYPLE_RESOURCE_LIB} pthread)

SET(USECASE9 UseCase9_ChangePin)
SET(USECASE9_PCSC ${USECASE9}_Pcsc)
ADD_EXECUTABLE(${USECASE9_PCSC}
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE9}/Main_ChangePin_Pcsc.cpp)
TARGET_LINK_LIBRARIES(${USECASE9_PCSC} ${KEYPLE_CARD_LIB} ${KEYPLE_PCSC_LIB} ${KEYPLE_SERVICE_LIB} ${KEYPLE_UTIL_LIB} ${KEYPLE_CALYPSO_LIB} ${KEYPLE_RESOURCE_LIB} pthread)

SET(USECASE10 UseCase10_SessionTrace_TN313)
SET(USECASE10_PCSC ${USECASE10}_Pcsc)
ADD_EXECUTABLE(${USECASE10_PCSC}
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE10}/CardReaderObserver.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE10}/Main_SessionTrace_TN313_Pcsc.cpp)
TARGET_LINK_LIBRARIES(${USECASE10_PCSC} ${KEYPLE_CARD_LIB} ${KEYPLE_PCSC_LIB} ${KEYPLE_SERVICE_LIB} ${KEYPLE_UTIL_LIB} ${KEYPLE_CALYPSO_LIB} ${KEYPLE_RESOURCE_LIB} pthread)
