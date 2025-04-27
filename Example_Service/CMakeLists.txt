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

SET(USECASE1 UseCase1_BasicSelection)
ADD_EXECUTABLE(

    ${USECASE1}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE1}/Main_BasicSelection_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE1}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Util
)

SET(USECASE2 UseCase2_ProtocolBasedSelection)
ADD_EXECUTABLE(

    ${USECASE2}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE2}/Main_ProtocolBasedSelection_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE2}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Util
)

SET(USECASE3 UseCase3_AidBasedSelection)
ADD_EXECUTABLE(

    ${USECASE3}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE3}/Main_AidBasedSelection_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE3}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Util
)

SET(USECASE4 UseCase4_ScheduledSelection)
ADD_EXECUTABLE(

    ${USECASE4}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE4}/CardReaderObserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE4}/Main_ScheduledSelection_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE4}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Util
)

SET(USECASE5 UseCase5_SequentialMultiSelection)
ADD_EXECUTABLE(

    ${USECASE5}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE5}/Main_SequentialMultiSelection_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE5}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Util
)

SET(USECASE6 UseCase6_GroupedMultiSelection)
ADD_EXECUTABLE(

    ${USECASE6}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE6}/Main_GroupedMultiSelection_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE6}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Util
)

SET(USECASE7 UseCase7_PluginAndReaderObservation)
ADD_EXECUTABLE(

    ${USECASE7}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/common/ConfigurationUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE7}/PluginObserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE7}/ReaderObserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE7}/Main_PluginAndReaderObservation_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE7}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Util
)
