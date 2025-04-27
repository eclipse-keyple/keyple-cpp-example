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
#include "keypop/reader/CardReaderEvent.hpp"
#include "keypop/reader/ObservableCardReader.hpp"
#include "keypop/reader/selection/CardSelectionManager.hpp"
#include "keypop/reader/spi/CardReaderObservationExceptionHandlerSpi.hpp"
#include "keypop/reader/spi/CardReaderObserverSpi.hpp"

using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keypop::reader::CardReaderEvent;
using keypop::reader::ObservableCardReader;
using keypop::reader::selection::CardSelectionManager;
using keypop::reader::spi::CardReaderObservationExceptionHandlerSpi;
using keypop::reader::spi::CardReaderObserverSpi;

/**
 * A reader Observer handles card event such as CARD_INSERTED, CARD_MATCHED,
 * CARD_REMOVED
 */
class CardReaderObserver : public CardReaderObserverSpi,
                           public CardReaderObservationExceptionHandlerSpi {
public:
    /**
     * Constructor.
     *
     * <p>Note: the reader is provided here for convenience but could also be
     * retrieved from the SmartCardService with its name and that of the plugin
     * both present in the CardReaderEvent.
     *
     * @param reader The card reader.
     * @param cardSelectionManager The card selection manager.
     */
    CardReaderObserver(
        std::shared_ptr<ObservableCardReader> reader,
        std::shared_ptr<CardSelectionManager> cardSelectionManager);

    /**
     *
     */
    virtual ~CardReaderObserver() = default;

    /**
     * {@inheritDoc}
     */
    void onReaderEvent(const std::shared_ptr<CardReaderEvent> event) override;

    /**
     * {@inheritDoc}
     */
    void onReaderObservationError(
        const std::string& pluginName,
        const std::string& readerName,
        const std::shared_ptr<std::exception> e) override;

private:
    /**
     *
     */
    const std::unique_ptr<Logger> mLogger
        = LoggerFactory::getLogger(typeid(CardReaderObserver));

    /**
     *
     */
    std::shared_ptr<ObservableCardReader> mReader;

    /**
     *
     */
    std::shared_ptr<CardSelectionManager> mCardSelectionManager;
};
