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

/* Keyple Core Util */
#include "ByteArrayUtil.h"
#include "StringUtils.h"

/* Keyple Cpp Examples */
#include "CalypsoConstants.h"

/* Keyple Core Service */
#include "ObservableReader.h"

using namespace calypsonet::terminal::calypso::card;
using namespace keyple::core::service;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp;

CardReaderObserver::CardReaderObserver(std::shared_ptr<CardReader> reader,
                                       std::shared_ptr<CardSelectionManager> cardSelectionManager)
: mReader(reader), mCardSelectionManager(cardSelectionManager) {}

void CardReaderObserver::onReaderEvent(const std::shared_ptr<CardReaderEvent> event)
{
    switch (event->getType()) {
    case CardReaderEvent::Type::CARD_MATCHED:
        {
        /* The selection has one target, get the result at index 0 */
        auto calypsoCard =
            std::dynamic_pointer_cast<CalypsoCard>(
                mCardSelectionManager->parseScheduledCardSelectionsResponse(
                                          event->getScheduledCardSelectionsResponse())
                                    ->getActiveSmartCard());

        mLogger->info("Observer notification: card selection was successful and produced the smart" \
                     " card = %\n",
                     calypsoCard);
        mLogger->info("Calypso Serial Number = %\n",
                     ByteArrayUtil::toHex(calypsoCard->getApplicationSerialNumber()));
        mLogger->info("Data read during the scheduled selection process:\n");
        mLogger->info("File %h, rec 1: FILE_CONTENT = %\n",
                     StringUtils::format("%02X", CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER),
                     calypsoCard->getFileBySfi(CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER));

        mLogger->info("= #### End of the card processing\n");
        }
        break;

    case CardReaderEvent::Type::CARD_INSERTED:
        mLogger->error("CARD_INSERTED event: should not have occurred because of the MATCHED_ONLY" \
                      " selection mode chosen.");
        break;

    case CardReaderEvent::Type::CARD_REMOVED:
        mLogger->trace("There is no card inserted anymore. Return to the waiting state...");
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
        std::dynamic_pointer_cast<ObservableReader>(mReader)->finalizeCardProcessing();
    }
}

void CardReaderObserver::onReaderObservationError(const std::string& pluginName,
                                                  const std::string& readerName,
                                                  const std::shared_ptr<Exception> e)
{
    mLogger->error("An exception occurred in plugin '%', reader '%'\n", pluginName, readerName, e);
}
