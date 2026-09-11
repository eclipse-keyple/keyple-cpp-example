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

#include "CardReaderObserver.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#include "keyple/card/calypso/CalypsoExtensionService.hpp"
#include "keyple/core/util/HexUtil.hpp"
#include "keypop/calypso/card/WriteAccessLevel.hpp"
#include "keypop/calypso/card/card/CalypsoCard.hpp"
#include "keypop/calypso/card/cpp/SecureRegularModeTransactionManagerBase.hpp"
#include "keypop/calypso/card/transaction/SecureSymmetricCryptoTransactionManager.hpp"
#include "keypop/reader/ChannelControl.hpp"
#include "keypop/reader/selection/spi/SmartCard.hpp"

using keyple::card::calypso::CalypsoExtensionService;
using keyple::core::util::HexUtil;
using keypop::calypso::card::WriteAccessLevel;
using keypop::calypso::card::card::CalypsoCard;
using keypop::calypso::card::cpp::SecureRegularModeTransactionManagerBase;
using keypop::calypso::card::transaction::
    SecureSymmetricCryptoTransactionManager;
using keypop::reader::ChannelControl;
using keypop::reader::selection::spi::SmartCard;

/* File identifiers */
static const std::uint8_t SFI_ENVIRONMENT_AND_HOLDER = 0x07;
static const std::uint8_t SFI_EVENT_LOG = 0x08;
static const std::uint8_t SFI_CONTRACT_LIST = 0x1E;
static const std::uint8_t SFI_CONTRACTS = 0x09;
static const int RECORD_SIZE = 29;

static const std::string ANSI_RESET = "\033[0m";
static const std::string ANSI_RED = "\033[31m";
static const std::string ANSI_GREEN = "\033[32m";

CardReaderObserver::CardReaderObserver(
    std::shared_ptr<Plugin> plugin,
    std::shared_ptr<ObservableCardReader> cardReader,
    std::shared_ptr<CardSelectionManager> cardSelectionManager,
    std::shared_ptr<SymmetricCryptoSecuritySetting> cardSecuritySetting)
: mPlugin(plugin)
, mCardReader(cardReader)
, mCardSelectionManager(cardSelectionManager)
, mCardSecuritySetting(cardSecuritySetting)
, mCalypsoCardApiFactory(
      CalypsoExtensionService::getInstance()->getCalypsoCardApiFactory())
, mNewEventRecord(
      HexUtil::toByteArray(
          "8013C8EC55667788112233445566778811223344556677881122334455")) {
}

void
CardReaderObserver::onReaderEvent(
    const std::shared_ptr<CardReaderEvent> event) {
    switch (event->getType()) {
    case CardReaderEvent::CARD_MATCHED: {
        /* Read the current time used later to compute the transaction time */
        const auto timeStamp = std::chrono::steady_clock::now();
        try {
            /* The selection matched, get the resulting CalypsoCard */
            const std::shared_ptr<SmartCard> smartCard(
                mCardSelectionManager
                    ->parseScheduledCardSelectionsResponse(
                        event->getScheduledCardSelectionsResponse())
                    ->getActiveSmartCard());
            auto calypsoCard
                = std::dynamic_pointer_cast<CalypsoCard>(smartCard);

            /*
             * Create a transaction manager, open a Secure Session, read
             * Environment, Event Log and Contract List.
             * Specifying expected response lengths in read commands serves
             * as a protective measure for legacy cards.
             *
             * The keypop API declares
             * createSecureRegularModeTransactionManager() as returning a
             * SecureRegularModeTransactionManagerBase, which does not itself
             * expose prepareOpenSecureSession() (it's only declared on
             * SecureSymmetricCryptoTransactionManager<T>, a sibling
             * interface implemented by the same concrete object). A
             * downcast is required to reach it.
             */
            std::unique_ptr<SecureRegularModeTransactionManagerBase>
                cardTransactionManagerBase(
                    mCalypsoCardApiFactory
                        ->createSecureRegularModeTransactionManager(
                            mCardReader, calypsoCard, mCardSecuritySetting));

            auto cardTransactionManager
                = dynamic_cast<SecureSymmetricCryptoTransactionManager<
                    SecureRegularModeTransactionManagerBase>*>(
                    cardTransactionManagerBase.get());

            cardTransactionManager
                ->prepareOpenSecureSession(WriteAccessLevel::DEBIT)
                .prepareReadRecords(
                    SFI_ENVIRONMENT_AND_HOLDER, 1, 1, RECORD_SIZE)
                .prepareReadRecords(SFI_EVENT_LOG, 1, 1, RECORD_SIZE)
                .prepareReadRecords(SFI_CONTRACT_LIST, 1, 1, RECORD_SIZE)
                .processCommands(ChannelControl::KEEP_OPEN);

            /* Place for the analysis of the context and the list of contracts
             */

            /*
             * Read the elected contract.
             * Specifying expected response lengths in read commands serves
             * as a protective measure for legacy cards.
             */
            cardTransactionManagerBase
                ->prepareReadRecords(SFI_CONTRACTS, 1, 1, RECORD_SIZE)
                .processCommands(ChannelControl::KEEP_OPEN);

            /* Place for the analysis of the contracts */

            /* Add an event record and close the Secure Session */
            cardTransactionManagerBase
                ->prepareAppendRecord(SFI_EVENT_LOG, mNewEventRecord)
                .prepareCloseSecureSession()
                .processCommands(ChannelControl::CLOSE_AFTER);

            /* Display transaction time */
            const auto elapsedMs
                = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - timeStamp)
                      .count();
            mLogger->info(
                "%Transaction succeeded. Execution time: % ms%\n",
                ANSI_GREEN,
                elapsedMs,
                ANSI_RESET);

            /* Optimization: preload the SAM challenge for the next transaction
             */
            mCardSecuritySetting->initCryptoContextForNextTransaction();

        } catch (const std::exception& e) {
            mLogger->error(
                "%Transaction failed with exception: % %\n",
                ANSI_RED,
                e.what(),
                ANSI_RESET);
        }
    } break;

    case CardReaderEvent::CARD_INSERTED:
        mLogger->error(
            "CARD_INSERTED event: should not have occurred because of the "
            "MATCHED_ONLY selection mode chosen\n");
        break;

    case CardReaderEvent::CARD_REMOVED:
        mLogger->info("Card removed\n");
        break;

    default:
        break;
    }

    if (event->getType() == CardReaderEvent::CARD_INSERTED
        || event->getType() == CardReaderEvent::CARD_MATCHED) {
        /*
         * Informs the underlying layer of the end of the card processing, in
         * order to manage the removal sequence.
         */
        mCardReader->finalizeCardProcessing();
    }
}

void
CardReaderObserver::onReaderObservationError(
    const std::string& pluginName,
    const std::string& readerName,
    const std::shared_ptr<std::exception> e) {
    mLogger->error(
        "An exception occurred in plugin '%', reader '%': %\n",
        pluginName,
        readerName,
        e ? e->what() : "unknown");
}
