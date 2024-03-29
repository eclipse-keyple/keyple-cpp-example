/**************************************************************************************************
 * Copyright (c) 2022 Calypso Networks Association https://calypsonet.org/                        *
 *                                                                                                *
 * See the NOTICE file(s) distributed with this work for additional information regarding         *
 * copyright ownership.                                                                           *
 *                                                                                                *
 * This program and the accompanying materials are made available under the terms of the Eclipse  *
 * Public License 2.0 which is available at http://www.eclipse.org/legal/epl-2.0                  *
 *                                                                                                *
 * SPDX-License-Identifier: EPL-2.0                                                               *
 **************************************************************************************************/

#include "StubSmartCardFactory.h"

#include "ConfigurationUtil.h"

/* Keyple Core Util */
#include "HexUtil.h"

/* Keyple Plugin Stub */
#include "StubSmartCard.h"

using namespace keyple::core::util;
using namespace keyple::plugin::stub;

const std::string StubSmartCardFactory::CARD_POWER_ON_DATA = "3B888001000000009171710098";
const std::string StubSmartCardFactory::SAM_POWER_ON_DATA =
    "3B3F9600805A0080C120000012345678829000";

std::shared_ptr<StubSmartCard> StubSmartCardFactory::mStubCard =
    StubSmartCard::builder()
        ->withPowerOnData(HexUtil::toByteArray(CARD_POWER_ON_DATA))
        .withProtocol(ConfigurationUtil::ISO_CARD_PROTOCOL)
        /* Select application */
        .withSimulatedCommand(
            "00A4040009315449432E4943413100",
            "6F238409315449432E49434131A516BF0C13C70800000000AABBCCDD53070A3C2305141" \
            "0019000")
        /* Read records */
        .withSimulatedCommand(
            "00B2013C00",
            "00112233445566778899AABBCCDDEEFF00112233445566778899AABBCC9000")
        /* Open secure session */
        .withSimulatedCommand(
            "008A0B39040011223300",
            "0308D1810030791D00112233445566778899AABBCCDDEEFF00112233445566778899AAB" \
            "BCC9000")
        /* Close secure session */
        .withSimulatedCommand("008E8000041234567800", "876543219000")
        /* Ping command (used by the card removal procedure) */
        .withSimulatedCommand("00C0000000", "9000")
        .build();

std::shared_ptr<StubSmartCard> StubSmartCardFactory::mStubSam =
    StubSmartCard::builder()
        ->withPowerOnData(HexUtil::toByteArray(SAM_POWER_ON_DATA))
        .withProtocol(ConfigurationUtil::SAM_PROTOCOL)
        /* Select diversifier */
        .withSimulatedCommand("801400000800000000AABBCCDD", "9000")
        /* Get challenge */
        .withSimulatedCommand("8084000004", "001122339000")
        /* Digest init */
        .withSimulatedCommand("808A00FF2730790308D1810030791D00112233445566778899AABBCCDDEEFF0011" \
                              "2233445566778899AABBCC",
                              "9000")
        /* Digest close */
        .withSimulatedCommand("808E000004", "123456789000")
        /* Digest authenticate */
        .withSimulatedCommand("808200000487654321", "9000")
        .build();

StubSmartCardFactory::StubSmartCardFactory() {}

std::shared_ptr<StubSmartCard> StubSmartCardFactory::getStubCard()
{
    return mStubCard;
}

std::shared_ptr<StubSmartCard> StubSmartCardFactory::getStubSam()
{
    return mStubSam;
}