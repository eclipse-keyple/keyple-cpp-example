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

/* Keyple Core Service */
#include "PluginObservationExceptionHandlerSpi.h"
#include "PluginObserverSpi.h"
#include "Reader.h"

/* Keyple Core Util */
#include "LoggerFactory.h"


/* Examples */
#include "ReaderObserver.h"

using namespace keyple::core::service;
using namespace keyple::core::service::spi;
using namespace keyple::core::util::cpp;

/**
 * (package-private)<br>
 * Implements the plugin observation SPIs. A plugin Observer to handle reader events such as
 * READER_CONNECTED or READER_DISCONNECTED.
 *
 * @since 2.0.0
 */
class PluginObserver : public PluginObserverSpi, public PluginObservationExceptionHandlerSpi {
public:
    /**
     * 
     */
    virtual ~PluginObserver() = default;
    
    /**
     * (package-private)<br>
     * Constructor.
     *
     * <p>Add an observer to all provided readers that are observable.
     *
     * @param initialReaders The readers connected before the plugin is observed.
     * @since 2.0.0
     */
    PluginObserver(const std::vector<std::shared_ptr<Reader>>& initialReaders);

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void onPluginEvent(const std::shared_ptr<PluginEvent> event) override;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void onPluginObservationError(const std::string& pluginName, const std::shared_ptr<Exception> e)
        override;

private:
    /**
     *
     */
    const std::unique_ptr<Logger> mLogger = LoggerFactory::getLogger(typeid(PluginObserver));

    /**
     *
     */
    std::shared_ptr<ReaderObserver> mReaderObserver;

    /**
     * Configure the reader to handle ISO14443-4 contactless cards
     *
     * @param reader The reader.
     */
    void setupReader(std::shared_ptr<Reader> reader);

    /**
     * Add the unique observer to the provided observable reader.
     *
     * @param reader An observable reader
     */
    void addObserver(std::shared_ptr<Reader> reader);
};