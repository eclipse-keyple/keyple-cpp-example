# *****************************************************************************
# Copyright (c) 2025 Calypso Networks Association https://calypsonet.org/     *
#                                                                             *
# See the NOTICE file(s) distributed with this work for additional            *
# information regarding copyright ownership.                                  *
#                                                                             *
# This program and the accompanying materials are made available under the    *
# terms of the Eclipse Distribution License 1.0 which is available at         *
# https://www.eclipse.org/org/documents/edl-v10.php                           *
#                                                                             *
# SPDX-License-Identifier: BSD-3-Clause                                       *
# *****************************************************************************/


INCLUDE_DIRECTORIES(

    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/
)

SET(USECASE1 UseCase1_ExplicitSelectionAid)
SET(USECASE1_STUB ${USECASE1}_Stub)
ADD_EXECUTABLE(

    ${USECASE1_STUB}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/StubSmartCardFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE1}/Main_ExplicitSelectionAid_Stub.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE1_STUB}

    Keyple::Card::Generic
    Keyple::Plugin::Stub
    Keyple::Service
    Keyple::Util
    Keyple::Card::Calypso
)

SET(USECASE1_PCSC ${USECASE1}_Pcsc)
ADD_EXECUTABLE(

    ${USECASE1_PCSC}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE1}/Main_ExplicitSelectionAid_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE1_PCSC}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Util
    Keyple::Card::Calypso
)

SET(USECASE2 UseCase2_ScheduledSelection)
SET(USECASE2_STUB ${USECASE2}_Stub)
ADD_EXECUTABLE(

    ${USECASE2_STUB}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/StubSmartCardFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE2}/CardReaderObserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE2}/Main_ScheduledSelection_Stub.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE2_STUB}

    Keyple::Card::Generic
    Keyple::Plugin::Stub
    Keyple::Service
    Keyple::Util
    Keyple::Card::Calypso
)

SET(USECASE2_PCSC ${USECASE2}_Pcsc)
ADD_EXECUTABLE(

    ${USECASE2_PCSC}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE2}/CardReaderObserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE2}/Main_ScheduledSelection_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE2_PCSC}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Util
    Keyple::Card::Calypso
)

SET(USECASE3 UseCase3_Rev1Selection)
SET(USECASE3_PCSC ${USECASE3}_Pcsc)
ADD_EXECUTABLE(

    ${USECASE3_PCSC}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE3}/Main_Rev1Selection_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE3_PCSC}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Util
    Keyple::Card::Calypso
)
SET(USECASE4 UseCase4_CardAuthentication)
SET(USECASE4_STUB ${USECASE4}_Stub)
ADD_EXECUTABLE(

    ${USECASE4_STUB}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/StubSmartCardFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE4}/Main_CardAuthentication_Stub.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE4_STUB}

    Keyple::Card::Generic
    Keyple::Plugin::Stub
    Keyple::Service
    Keyple::Service::Resource
    Keyple::Util
    Keyple::Card::Calypso
    Keyple::Card::Calypso::Crypto::LegacySam
)

SET(USECASE4_PCSC ${USECASE4}_Pcsc)
ADD_EXECUTABLE(

    ${USECASE4_PCSC}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE4}/Main_CardAuthentication_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE4_PCSC}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Service::Resource
    Keyple::Util
    Keyple::Card::Calypso
    Keyple::Card::Calypso::Crypto::LegacySam
)

SET(USECASE4_PCSC_SAM_RESOURCE ${USECASE4}_Pcsc_SamResourceService)
ADD_EXECUTABLE(

    ${USECASE4_PCSC_SAM_RESOURCE}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE4}/Main_CardAuthentication_Pcsc_SamResourceService.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE4_PCSC_SAM_RESOURCE}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Service::Resource
    Keyple::Util
    Keyple::Card::Calypso
    Keyple::Card::Calypso::Crypto::LegacySam
)
SET(USECASE5 UseCase5_MultipleSession)
SET(USECASE5_PCSC ${USECASE5}_Pcsc)
ADD_EXECUTABLE(

    ${USECASE5_PCSC}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE5}/Main_MultipleSession_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE5_PCSC}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Service::Resource
    Keyple::Util
    Keyple::Card::Calypso
    Keyple::Card::Calypso::Crypto::LegacySam
)

SET(USECASE6 UseCase6_VerifyPin)
SET(USECASE6_PCSC ${USECASE6}_Pcsc)
ADD_EXECUTABLE(

    ${USECASE6_PCSC}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE6}/Main_VerifyPin_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE6_PCSC}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Service::Resource
    Keyple::Util
    Keyple::Card::Calypso
    Keyple::Card::Calypso::Crypto::LegacySam
)

SET(USECASE7 UseCase7_StoredValue_SimpleReloading)
SET(USECASE7_PCSC ${USECASE7}_Pcsc)
ADD_EXECUTABLE(

    ${USECASE7_PCSC}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE7}/Main_StoredValue_SimpleReloading_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE7_PCSC}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Service::Resource
    Keyple::Util
    Keyple::Card::Calypso
    Keyple::Card::Calypso::Crypto::LegacySam
)

SET(USECASE8 UseCase8_StoredValue_DebitInSession)
SET(USECASE8_PCSC ${USECASE8}_Pcsc)
ADD_EXECUTABLE(

    ${USECASE8_PCSC}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE8}/Main_StoredValue_DebitInSession_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE8_PCSC}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Service::Resource
    Keyple::Util
    Keyple::Card::Calypso
    Keyple::Card::Calypso::Crypto::LegacySam
)

SET(USECASE9 UseCase9_ChangePin)
SET(USECASE9_PCSC ${USECASE9}_Pcsc)
ADD_EXECUTABLE(

    ${USECASE9_PCSC}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE9}/Main_ChangePin_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE9_PCSC}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Service::Resource
    Keyple::Util
    Keyple::Card::Calypso
    Keyple::Card::Calypso::Crypto::LegacySam
)

SET(USECASE10 UseCase10_SessionTrace_TN313)
SET(USECASE10_PCSC ${USECASE10}_Pcsc)
ADD_EXECUTABLE(

    ${USECASE10_PCSC}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE10}/CardReaderObserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE10}/Main_SessionTrace_TN313_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE10_PCSC}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Service::Resource
    Keyple::Util
    Keyple::Card::Calypso
    Keyple::Card::Calypso::Crypto::LegacySam
)

SET(USECASE11 UseCase11_DataSigning)
SET(USECASE11_PCSC ${USECASE11}_Pcsc)
ADD_EXECUTABLE(

    ${USECASE11_PCSC}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE11}/Main_DataSigning_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE11_PCSC}

    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Service::Resource
    Keyple::Util
    Keyple::Card::Calypso
    Keyple::Card::Calypso::Crypto::LegacySam
)

SET(USECASE12 UseCase12_PerformanceMeasurement_EmbeddedValidation)
SET(USECASE12_PCSC ${USECASE12}_Pcsc)
ADD_EXECUTABLE(

    ${USECASE12_PCSC}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE12}/Main_PerformanceMeasurement_EmbeddedValidation_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE12_PCSC}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Service::Resource
    Keyple::Util
    Keyple::Card::Calypso
    Keyple::Card::Calypso::Crypto::LegacySam
)

SET(USECASE13 UseCase13_PerformanceMeasurement_DistributedReloading)
SET(USECASE13_PCSC ${USECASE13}_Pcsc)
ADD_EXECUTABLE(

    ${USECASE13_PCSC}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/CalypsoConstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE13}/Main_PerformanceMeasurement_DistributedReloading_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE13_PCSC}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Service::Resource
    Keyple::Util
    Keyple::Card::Calypso
    Keyple::Card::Calypso::Crypto::LegacySam
)
