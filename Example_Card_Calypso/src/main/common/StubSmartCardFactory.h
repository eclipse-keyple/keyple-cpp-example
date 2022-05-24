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

#include <memory>
#include <string>

/* Keyple Plugin Stub */
#include "StubSmartCard.h"

using namespace keyple::plugin::stub;

/**
 * Factory for a Calypso Card emulation via a smart card stub
 */
class StubSmartCardFactory {
public:
    /**
     * Get the stub smart card for a Calypso card
     *
     * @return A not null reference
     */
    static std::shared_ptr<StubSmartCard> getStubCard();

    /**
     * Get the stub smart card for a Calypso SAM
     *
     * @return A not null reference
     */
    static std::shared_ptr<StubSmartCard> getStubSam();

private:
    /**
     *
     */
    static const std::string CARD_POWER_ON_DATA;

    /**
     *
     */
    static const std::string CARD_PROTOCOL;

    /**
     *
     */
    static std::shared_ptr<StubSmartCard> mStubCard;

    /**
     *
     */
    static const std::string SAM_POWER_ON_DATA;

    /**
     *
     */
    static const std::string SAM_PROTOCOL;

    /**
     *
     */
    static std::shared_ptr<StubSmartCard> mStubSam;

    /**
     * (private)<br>
     * Constructor
     */
    StubSmartCardFactory();
};