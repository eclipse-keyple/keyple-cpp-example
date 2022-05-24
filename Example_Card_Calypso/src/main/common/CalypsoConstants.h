/**************************************************************************************************
 * Copyright (c) 2021 Calypso Networks Association https://calypsonet.org/                        *
 *                                                                                                *
 * See the NOTICE file(s) distributed with this work for additional information regarding         *
 * copyright ownership.                                                                           *
 *                                                                                                *
 * This program and the accompanying materials are made available under the terms of the Eclipse  *
 * Public License 2.0 which is available at http://www.eclipse.org/legal/epl-2.0                  *
 *                                                                                                *
 * SPDX-License-Identifier: EPL-2.0                                                               *
 **************************************************************************************************/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

/**
 * Helper class to provide specific constants to manipulate Calypso cards from the Keyple demo kit.
 *
 * <ul>
 *   <li>AID application selection (default Calypso AID)
 *   <li>File
 *   <li>File definitions and identifiers (SFI)
 *   <li>Sample data
 *   <li>Security settings
 * </ul>
 */
class CalypsoConstants final {
public:
    /* Application */

    /**
     * AID: Keyple test kit profile 1, Application 2
     */
    static const std::string AID;

    /* File structure */

    static const int RECORD_SIZE;
    static const uint8_t RECORD_NUMBER_1;
    static const uint8_t RECORD_NUMBER_2;
    static const uint8_t RECORD_NUMBER_3;
    static const uint8_t RECORD_NUMBER_4;

    /* File identifiers */
    static const uint8_t SFI_ENVIRONMENT_AND_HOLDER;
    static const uint8_t SFI_EVENT_LOG;
    static const uint8_t SFI_CONTRACT_LIST;
    static const uint8_t SFI_CONTRACTS;
    static const uint16_t LID_DF_RT;
    static const uint16_t LID_EVENT_LOG;

    /* Sample data */
    static const std::string EVENT_LOG_DATA_FILL;

    /* Security settings */
    static const std::string SAM_PROFILE_NAME;
    static const uint8_t PIN_MODIFICATION_CIPHERING_KEY_KIF;
    static const uint8_t PIN_MODIFICATION_CIPHERING_KEY_KVC;
    static const uint8_t PIN_VERIFICATION_CIPHERING_KEY_KIF;
    static const uint8_t PIN_VERIFICATION_CIPHERING_KEY_KVC;
    static const std::vector<uint8_t> PIN_OK;
    static const std::vector<uint8_t> PIN_KO;

private:
    /**
     * (private)<br>
     * Constructor.
     */
    CalypsoConstants();
};
