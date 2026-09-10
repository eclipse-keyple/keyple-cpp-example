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

#include "keyple/core/service/Plugin.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keypop/reader/CardReader.hpp"

using keyple::core::service::Plugin;
using keyple::plugin::pcsc::PcscReader;
using keypop::reader::CardReader;

/**
 * Utility class providing methods for configuring PC/SC readers used across
 * the Calypso card examples.
 *
 * <p>The two regular expressions below can be edited to match the names of
 * the PC/SC readers actually used to run these examples (contactless reader
 * for the Calypso card, contact reader for the Calypso SAM).
 */
class ConfigurationUtil final {
public:
    /**
     *
     */
    static const std::string ISO_CARD_PROTOCOL;

    /**
     *
     */
    static const std::string INNOVATRON_CARD_PROTOCOL;

    /**
     *
     */
    static const std::string SAM_PROTOCOL;

    /**
     *
     */
    static const std::string CARD_READER_NAME_REGEX;

    /**
     *
     */
    static const std::string SAM_READER_NAME_REGEX;

    /**
     * Configures and returns a card reader based on the provided parameters.
     *
     * <p>It finds the reader name by matching with a regular expression, then
     * configures the reader with the specified settings.
     *
     * @param plugin The plugin used to interact with the card reader.
     * @param readerNameRegex The regular expression to match the card
     * reader's name.
     * @param isContactless A boolean indicating whether the card reader is
     * contactless.
     * @param isoProtocol The ISO protocol used by the card reader.
     * @param sharingMode The sharing mode of the PC/SC reader.
     * @param physicalProtocolName The name of the protocol used by the reader
     * to communicate with the card.
     * @param logicalProtocolName The name of the protocol known by the
     * application.
     * @return The configured card reader.
     */
    static std::shared_ptr<CardReader> getReader(
        std::shared_ptr<Plugin> plugin,
        const std::string& readerNameRegex,
        bool isContactless,
        const PcscReader::IsoProtocol& isoProtocol,
        const PcscReader::SharingMode sharingMode,
        const std::string& physicalProtocolName,
        const std::string& logicalProtocolName);

private:
    /**
     * Constructor.
     */
    ConfigurationUtil() = default;
};
