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

#pragma once

/* Calypsonet Terminal Reader */
#include "CardReader.h"

/* Keyple Core Service */
#include "Plugin.h"
#include "SmartCardServiceProvider.h"

/* Keyple Core Util */
#include "LoggerFactory.h"

/* Keyple Service Resource */
#include "ReaderConfiguratorSpi.h"

using namespace calypsonet::terminal::reader;
using namespace keyple::core::service;
using namespace keyple::core::service::resource::spi;
using namespace keyple::core::util::cpp;

/**
 * Utility class providing methods for configuring readers and the card resource service used across
 * several examples.
 */
class ConfigurationUtil {
public:
    /**
     * Common reader identifiers
     * These two regular expressions can be modified to fit the names of the readers used to run
     * these examples.
     */
    static const std::string CARD_READER_NAME_REGEX;
    static const std::string SAM_READER_NAME_REGEX;
    static const std::string SAM_PROTOCOL;
    static const std::string ISO_CARD_PROTOCOL;
    static const std::string INNOVATRON_CARD_PROTOCOL;

    /**
     * Retrieves the name of the first available reader in the provided plugin whose name matches
     * the provided regular expression.
     *
     * @param plugin The plugin to which the reader belongs.
     * @param readerNameRegex A regular expression matching the targeted reader.
     * @return The name of the found reader.
     * @throw IllegalStateException If the reader is not found.
     */
    static const std::string& getCardReaderName(std::shared_ptr<Plugin> plugin,
                                                const std::string& readerNameRegex);
    /**
     * Set up the CardResourceService to provide a Calypso SAM C1 resource when requested.
     *
     * @param plugin The plugin to which the SAM reader belongs.
     * @param readerNameRegex A regular expression matching the expected SAM reader name.
     * @param samProfileName A string defining the SAM profile.
     * @throw IllegalStateException If the expected card resource is not found.
     */
    static void setupCardResourceService(std::shared_ptr<Plugin> plugin,
                                         const std::string& readerNameRegex,
                                         const std::string& samProfileName);

private:
    /*
     * Reader configurator used by the card resource service to set up the SAM reader with the
     * required settings.
     */
    class ReaderConfigurator final : public ReaderConfiguratorSpi {
    public:
        friend class ConfigurationUtil;

        /**
         * {@inheritDoc}
         */
        void setupReader(std::shared_ptr<CardReader> cardReader) override;

    private:
        /**
         *
         */
        const std::unique_ptr<Logger> mLogger =
            LoggerFactory::getLogger(typeid(ReaderConfigurator));

        /**
         * (private)<br>
         * Constructor.
         */
        ReaderConfigurator();
    };

    /**
     *
     */
    static const std::unique_ptr<Logger> mLogger;

    /**
     * (private)<br>
     * Constructor.
     */
    ConfigurationUtil();
};
