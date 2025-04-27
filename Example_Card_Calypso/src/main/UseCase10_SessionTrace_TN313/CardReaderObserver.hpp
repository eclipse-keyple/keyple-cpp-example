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

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "keyple/core/service/Plugin.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keypop/calypso/card/CalypsoCardApiFactory.hpp"
#include "keypop/calypso/card/transaction/SymmetricCryptoSecuritySetting.hpp"
#include "keypop/reader/CardReaderEvent.hpp"
#include "keypop/reader/ObservableCardReader.hpp"
#include "keypop/reader/selection/CardSelectionManager.hpp"
#include "keypop/reader/spi/CardReaderObservationExceptionHandlerSpi.hpp"
#include "keypop/reader/spi/CardReaderObserverSpi.hpp"

using keyple::core::service::Plugin;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keypop::calypso::card::CalypsoCardApiFactory;
using keypop::calypso::card::transaction::SymmetricCryptoSecuritySetting;
using keypop::reader::CardReaderEvent;
using keypop::reader::ObservableCardReader;
using keypop::reader::selection::CardSelectionManager;
using keypop::reader::spi::CardReaderObservationExceptionHandlerSpi;
using keypop::reader::spi::CardReaderObserverSpi;

/**
 * A reader Observer handles card event such as CARD_INSERTED, CARD_MATCHED,
 * CARD_REMOVED. Executes the TN313 Secure Session transaction scenario upon a
 * CARD_MATCHED event.
 */
class CardReaderObserver final
: public CardReaderObserverSpi,
  public CardReaderObservationExceptionHandlerSpi {
public:
    /**
     * Constructor.
     *
     * @param plugin The plugin.
     * @param cardReader The card reader.
     * @param cardSelectionManager The card selection manager.
     * @param cardSecuritySetting The card security settings.
     */
    CardReaderObserver(
        std::shared_ptr<Plugin> plugin,
        std::shared_ptr<ObservableCardReader> cardReader,
        std::shared_ptr<CardSelectionManager> cardSelectionManager,
        std::shared_ptr<SymmetricCryptoSecuritySetting> cardSecuritySetting);

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
    std::shared_ptr<Plugin> mPlugin;

    /**
     *
     */
    std::shared_ptr<ObservableCardReader> mCardReader;

    /**
     *
     */
    std::shared_ptr<CardSelectionManager> mCardSelectionManager;

    /**
     *
     */
    std::shared_ptr<SymmetricCryptoSecuritySetting> mCardSecuritySetting;

    /**
     *
     */
    std::shared_ptr<CalypsoCardApiFactory> mCalypsoCardApiFactory;

    /**
     *
     */
    const std::vector<std::uint8_t> mNewEventRecord;
};
