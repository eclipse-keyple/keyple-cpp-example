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

/* Keyple Core Service */
#include "ObservableReader.h"

using namespace keyple::core::service;

CardReaderObserver::CardReaderObserver(
  std::shared_ptr<Reader> reader, std::shared_ptr<CardSelectionManager> cardSelectionManager)
: mReader(reader), mCardSelectionManager(cardSelectionManager) {}

void CardReaderObserver::onReaderEvent(const std::shared_ptr<CardReaderEvent> event)
{
    switch (event->getType()) {
    case CardReaderEvent::CARD_MATCHED:
        {
        /* The selection has one target, get the result at index 0 */
        const std::shared_ptr<SmartCard> smartCard =
            mCardSelectionManager->parseScheduledCardSelectionsResponse(
                event->getScheduledCardSelectionsResponse())
                ->getActiveSmartCard();

        mLogger->info("Observer notification: the selection of the card has succeeded and return " \
                      "the SmartCard = %\n",
                      smartCard);

        mLogger->info("= #### End of the card processing\n");
        }
        break;
    case CardReaderEvent::CARD_INSERTED:
        mLogger->error("CARD_INSERTED event: should not have occurred due to the MATCHED_ONLY " \
                       "selection mode\n");
        break;
    case CardReaderEvent::CARD_REMOVED:
        mLogger->trace("There is no card inserted anymore. Return to the waiting state...\n");
        break;
    default:
        break;
    }

    if (event->getType() == CardReaderEvent::CARD_INSERTED ||
        event->getType() == CardReaderEvent::CARD_MATCHED) {

        /*
        * Informs the underlying layer of the end of the card processing, in order to manage the
        * removal sequence.
        */
        std::dynamic_pointer_cast<ObservableReader>(mReader)->finalizeCardProcessing();
    }
}

void CardReaderObserver::onReaderObservationError(const std::string& pluginName,
                                                  const std::string& readerName, 
                                                  const std::shared_ptr<Exception> e)
{
    mLogger->error("An exception occurred in plugin '%', reader '%'\n", pluginName, readerName, e);
}