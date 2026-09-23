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
 * Implements the reader observation SPIs.<br>
 * A reader Observer to handle card events such as CARD_INSERTED, CARD_MATCHED,
 * CARD_REMOVED
 *
 * @since 2.0.0.0
 */
class CardReaderObserver : public CardReaderObserverSpi,
                           public CardReaderObservationExceptionHandlerSpi {
public:
    /**
     *
     */
    CardReaderObserver(
        std::shared_ptr<ObservableCardReader> observableCardReader,
        std::shared_ptr<CardSelectionManager> cardSelectionManager);

    /**
     *
     */
    virtual ~CardReaderObserver() = default;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void onReaderEvent(const std::shared_ptr<CardReaderEvent> event) override;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    void onReaderObservationError(
        const std::string& contextInfo,
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
    std::shared_ptr<ObservableCardReader> mObservableCardReader;

    /**
     *
     */
    std::shared_ptr<CardSelectionManager> mCardSelectionManager;
};
