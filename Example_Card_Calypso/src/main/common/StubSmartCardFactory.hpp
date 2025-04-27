/* ****************************************************************************
 * Copyright (c) 2025 Calypso Networks Association https://calypsonet.org/    *
 *                                                                            *
 * See the NOTICE file(s) distributed with this work for additional           *
 * information regarding copyright ownership.                                 *
 *                                                                            *
 * This program and the accompanying materials are made available under the   *
 * terms of the Eclipse Distribution License 1.0 which is available at        *
 * https://www.eclipse.org/org/documents/edl-v10.php                          *
 *                                                                            *
 * SPDX-License-Identifier: BSD-3-Clause                                      *
 ******************************************************************************/

#pragma once

#include <memory>
#include <string>

#include "keyple/plugin/stub/StubSmartCard.hpp"

using keyple::plugin::stub::StubSmartCard;

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
    static const std::string SAM_POWER_ON_DATA;
    static const std::string ISO_CARD_PROTOCOL;
    static const std::string SAM_PROTOCOL;

    /**
     *
     */
    static std::shared_ptr<StubSmartCard> mStubCard;

    /**
     *
     */
    static std::shared_ptr<StubSmartCard> mStubSam;

    /**
     * Constructor
     */
    StubSmartCardFactory();
};
