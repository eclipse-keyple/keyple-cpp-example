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

INCLUDE_DIRECTORIES(

)

SET(USECASE1 UseCase1_CardResourceService)
SET(USECASE1_STUB ${USECASE1}_Stub)
ADD_EXECUTABLE(

    ${USECASE1_STUB}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/${USECASE1}/Main_CardResourceService_Stub.cpp
)
TARGET_LINK_LIBRARIES(

    ${USECASE1_STUB}

    Keyple::Card::Generic
    Keyple::Plugin::Stub
    Keyple::Service
    Keyple::Service::Resource
    Keyple::Util
)

IF(APPLE)
    TARGET_LINK_LIBRARIES(${USECASE1_STUB} pthread)
ELSEIF(UNIX)
    TARGET_LINK_LIBRARIES(${USECASE1_STUB} pthread)
ENDIF(APPLE)
