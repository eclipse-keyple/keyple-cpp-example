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

)

SET(USECASE1 UseCase1_ReaderTypeAutoIdentification)
ADD_EXECUTABLE(

    ${USECASE1}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE1}/Main_ReaderTypeAutoIdentification_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE1}

    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Util
)

SET(USECASE2 UseCase2_ExplicitReaderType)
ADD_EXECUTABLE(

    ${USECASE2}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE2}/Main_ExplicitReaderType_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE2}

    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Util
)

SET(USECASE3 UseCase3_ChangeProtocolRules)
ADD_EXECUTABLE(

    ${USECASE3}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE3}/Main_ChangeProtocolRules_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE3}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Util
)

SET(USECASE4 UseCase4_TransmitControl)
ADD_EXECUTABLE(

    ${USECASE4}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE4}/Main_TransmitControl_Pcsc.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE4}

    Keyple::Card::Generic
    Keyple::Plugin::Pcsc
    Keyple::Service
    Keyple::Util
)
