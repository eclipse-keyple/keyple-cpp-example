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

/* Keyple Core Util */
#include "LoggerFactory.h"

/* Keyple Core Service */
#include "Plugin.h"
#include "Reader.h"

using namespace keyple::core::service;
using namespace keyple::core::util::cpp;

/**
 * Utility class providing methods for configuring readers and the card resource service used across
 * several examples.
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

    /*
     * Common reader identifiers
     * These two regular expressions can be modified to fit the names of the readers used to run
     * these examples.
     */
    static const std::string CONTACTLESS_READER_NAME_REGEX;
    static const std::string CONTACT_READER_NAME_REGEX;

    /**
     * Retrieves the first available reader in the provided plugin whose name matches the provided
     * regular expression.
     *
     * @param plugin The plugin to which the reader belongs.
     * @param readerNameRegex A regular expression matching the targeted reader.
     * @return A not null reference.
     * @throws IllegalStateException If the reader is not found.
     * @since 2.0.0
     */
    static std::shared_ptr<Reader> getCardReader(std::shared_ptr<Plugin> plugin, 
                                                 const std::string& readerNameRegex);

private:
    /**
     * 
     */
    static const std::unique_ptr<Logger> mLogger;

    /**
     * (private)<br>
     * Constructor.
     */
    ConfigurationUtil() {}
};
