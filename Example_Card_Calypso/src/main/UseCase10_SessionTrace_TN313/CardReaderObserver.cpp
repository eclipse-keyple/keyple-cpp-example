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

#include "CardReaderObserver.h"

/* Calypsonet Terminal Calypso */
#include "CalypsoCard.h"
#include "CardTransactionManager.h"

/* Keyple Card Calypso */
#include "CalypsoExtensionService.h"

/* Keyple Core Service */
#include "ObservableReader.h"

/* Keyple Core Util */
#include "System.h"

/* Keyple Cpp Example */
#include "CalypsoConstants.h"

using namespace calypsonet::terminal::calypso::card;
using namespace calypsonet::terminal::calypso::transaction;
using namespace keyple::card::calypso;
using namespace keyple::core::service;
using namespace keyple::core::util::cpp;

CardReaderObserver::CardReaderObserver(std::shared_ptr<CardReader> cardReader,
                                       std::shared_ptr<CardSelectionManager> cardSelectionManager,
                                       std::shared_ptr<CardSecuritySetting> cardSecuritySetting)
: mCardReader(cardReader),
  mCardSecuritySetting(cardSecuritySetting),
  mCardSelectionManager(cardSelectionManager) {}

void CardReaderObserver::onReaderEvent(const std::shared_ptr<CardReaderEvent> event)
{
    switch (event->getType()) {
    case CardReaderEvent::Type::CARD_MATCHED:
        {
        /* Read the current time used later to compute the transaction time */
        const long timeStamp = System::currentTimeMillis();

        try {
            /* The selection matched, get the resulting CalypsoCard */
            auto calypsoCard =
                std::dynamic_pointer_cast<CalypsoCard>(
                    mCardSelectionManager
                        ->parseScheduledCardSelectionsResponse(
                            event->getScheduledCardSelectionsResponse())
                        ->getActiveSmartCard());

            /*
             * Create a transaction manager, open a Secure Session, read Environment, Event Log and
             * Contract List.
             */
            std::shared_ptr<CardTransactionManager> cardTransactionManager =
                CalypsoExtensionService::getInstance()->createCardTransaction(mCardReader,
                                                                              calypsoCard,
                                                                              mCardSecuritySetting);
            cardTransactionManager->prepareReadRecord(CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER,
                                                      CalypsoConstants::RECORD_NUMBER_1)
                                   .prepareReadRecord(CalypsoConstants::SFI_EVENT_LOG,
                                                      CalypsoConstants::RECORD_NUMBER_1)
                                   .prepareReadRecord(CalypsoConstants::SFI_CONTRACT_LIST,
                                                      CalypsoConstants::RECORD_NUMBER_1)
                                   .processOpening(WriteAccessLevel::DEBIT);

            /*
             * Place for the analysis of the context and the list of contracts
             */

            /* Read the elected contract */
            cardTransactionManager->prepareReadRecord(CalypsoConstants::SFI_CONTRACTS,
                                                      CalypsoConstants::RECORD_NUMBER_1)
                                   .processCardCommands();

            /*
             * Place for the analysis of the contracts
             */

            /* Add an event record and close the Secure Session */
            cardTransactionManager->prepareAppendRecord(CalypsoConstants::SFI_EVENT_LOG,
                                                        mNewEventRecord)
                                   .processClosing();

            /* Display transaction time */
            mLogger->info("Transaction succeeded. Execution time: % ms\n",
                          System::currentTimeMillis() - timeStamp);
        } catch (const Exception& e) {
            mLogger->error("Transaction failed with exception: % - %\n", e.getMessage(), e);
        }
        }
        break;

    case CardReaderEvent::Type::CARD_INSERTED:
        mLogger->error("CARD_INSERTED event: should not have occurred because of the MATCHED_ONLY " \
                       "selection mode chosen\n");
        break;

    case CardReaderEvent::Type::CARD_REMOVED:
        mLogger->info("Card removed\n");
        break;
    default:
        break;
    }

    if (event->getType() == CardReaderEvent::Type::CARD_INSERTED ||
        event->getType() == CardReaderEvent::Type::CARD_MATCHED) {

        /*
        * Informs the underlying layer of the end of the card processing, in order to manage the
        * removal sequence.
        */
       std::dynamic_pointer_cast<ObservableReader>(mCardReader)->finalizeCardProcessing();
    }
}

void CardReaderObserver::onReaderObservationError(const std::string& pluginName,
                                                  const std::string& readerName,
                                                  const std::shared_ptr<Exception> e)
{
    mLogger->error("An exception occurred in plugin '%', reader '%'\n", pluginName, readerName, e);
}
