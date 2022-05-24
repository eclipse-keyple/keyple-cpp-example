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

#include "CalypsoConstants.h"

const std::string CalypsoConstants::AID = "315449432E49434131";

const int CalypsoConstants::RECORD_SIZE = 29;
const uint8_t CalypsoConstants::RECORD_NUMBER_1 = 1;
const uint8_t CalypsoConstants::RECORD_NUMBER_2 = 2;
const uint8_t CalypsoConstants::RECORD_NUMBER_3 = 3;
const uint8_t CalypsoConstants::RECORD_NUMBER_4 = 4;

const uint8_t CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER = 0x07;
const uint8_t CalypsoConstants::SFI_EVENT_LOG = 0x08;
const uint8_t CalypsoConstants::SFI_CONTRACT_LIST = 0x1E;
const uint8_t CalypsoConstants::SFI_CONTRACTS = 0x09;
const uint16_t CalypsoConstants::LID_DF_RT = 0x2000;
const uint16_t CalypsoConstants::LID_EVENT_LOG = 0x2010;

const std::string CalypsoConstants::EVENT_LOG_DATA_FILL =
        "00112233445566778899AABBCCDDEEFF00112233445566778899AABBCC";

const std::string CalypsoConstants::SAM_PROFILE_NAME = "SAM C1";
const uint8_t CalypsoConstants::PIN_MODIFICATION_CIPHERING_KEY_KIF = 0x21;
const uint8_t CalypsoConstants::PIN_MODIFICATION_CIPHERING_KEY_KVC = 0x79;
const uint8_t CalypsoConstants::PIN_VERIFICATION_CIPHERING_KEY_KIF = 0x30;
const uint8_t CalypsoConstants::PIN_VERIFICATION_CIPHERING_KEY_KVC = 0x79;
const std::vector<uint8_t> CalypsoConstants::PIN_OK = {0x30, 0x30, 0x30, 0x30};
const std::vector<uint8_t> CalypsoConstants::PIN_KO = {0x30, 0x30, 0x30, 0x31};

CalypsoConstants::CalypsoConstants() {}