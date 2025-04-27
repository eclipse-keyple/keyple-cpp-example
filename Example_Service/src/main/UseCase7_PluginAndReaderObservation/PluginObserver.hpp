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

#include "ReaderObserver.hpp"

#include "keyple/core/service/PluginEvent.hpp"
#include "keyple/core/service/spi/PluginObservationExceptionHandlerSpi.hpp"
#include "keyple/core/service/spi/PluginObserverSpi.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keyple/core/util/cpp/exception/Exception.hpp"
#include "keypop/reader/CardReader.hpp"

using keyple::core::service::PluginEvent;
using keyple::core::service::spi::PluginObservationExceptionHandlerSpi;
using keyple::core::service::spi::PluginObserverSpi;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keyple::core::util::cpp::exception::Exception;
using keypop::reader::CardReader;

/**
 * Implements the plugin observation SPIs. A plugin Observer to handle reader
 * events such as READER_CONNECTED or READER_DISCONNECTED.
 *
 * @since 2.0.0
 */
class PluginObserver : public PluginObserverSpi,
                       public PluginObservationExceptionHandlerSpi {
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
     * @param initialReaders The readers connected before the plugin is
     * observed.
     * @since 2.0.0
     */
    explicit PluginObserver(
        const std::vector<std::shared_ptr<CardReader>>& initialReaders);

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
    void onPluginObservationError(
        const std::string& pluginName,
        const std::shared_ptr<Exception> e) override;

private:
    /**
     *
     */
    const std::unique_ptr<Logger> mLogger
        = LoggerFactory::getLogger(typeid(PluginObserver));

    /**
     *
     */
    std::shared_ptr<ReaderObserver> mReaderObserver;

    /**
     * Configure the reader to handle ISO14443-4 contactless cards
     *
     * @param reader The reader.
     */
    void setupReader(std::shared_ptr<CardReader> cardReader);

    /**
     * Add the unique observer to the provided observable reader.
     *
     * @param reader An observable reader
     */
    void addObserver(std::shared_ptr<CardReader> reader);
};
