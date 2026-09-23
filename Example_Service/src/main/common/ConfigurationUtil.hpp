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

#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"

using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;

/**
 * Utility class providing methods for configuring readers and the card resource
 * service used across several examples.
 *
 * @since 2.0.0
 */
class ConfigurationUtil final {
public:
    /**
     *
     */
    static const std::string AID_EMV_PPSE;

    /**
     *
     */
    static const std::string AID_KEYPLE_PREFIX;

    /**
     *
     */
    static const std::string ISO_CARD_PROTOCOL;

    /*
     * Common reader identifiers
     * These two regular expressions can be modified to fit the names of the
     * readers used to run these examples.
     */
    static const std::string CONTACTLESS_READER_NAME_REGEX;
    static const std::string CONTACT_READER_NAME_REGEX;

private:
    /**
     *
     */
    static const std::unique_ptr<Logger> mLogger;

    /**
     * Constructor.
     */
    ConfigurationUtil() {
    }
};
