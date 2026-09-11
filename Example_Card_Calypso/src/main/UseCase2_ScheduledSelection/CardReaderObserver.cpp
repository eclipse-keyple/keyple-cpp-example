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

#include <memory>
#include <string>

#include "keyple/core/util/HexUtil.hpp"
#include "keypop/calypso/card/card/CalypsoCard.hpp"

using keyple::core::util::HexUtil;
using keypop::calypso::card::card::CalypsoCard;

/* File identifiers */
static const std::uint8_t SFI_ENVIRONMENT_AND_HOLDER = 0x07;

CardReaderObserver::CardReaderObserver(
    std::shared_ptr<ObservableCardReader> reader,
    std::shared_ptr<CardSelectionManager> cardSelectionManager)
: mReader(reader)
, mCardSelectionManager(cardSelectionManager) {
}

void
CardReaderObserver::onReaderEvent(
    const std::shared_ptr<CardReaderEvent> event) {
    switch (event->getType()) {
    case CardReaderEvent::CARD_MATCHED: {
        auto calypsoCard = std::dynamic_pointer_cast<CalypsoCard>(
            mCardSelectionManager
                ->parseScheduledCardSelectionsResponse(
                    event->getScheduledCardSelectionsResponse())
                ->getActiveSmartCard());

        mLogger->info(
            "Observer notification: card selection was successful and "
            "produced the smart card = %\n",
            calypsoCard);
        mLogger->info(
            "Calypso Serial Number = %\n",
            HexUtil::toHex(calypsoCard->getApplicationSerialNumber()));
        mLogger->info("Data read during the scheduled selection process:\n");
        mLogger->info(
            "File %h, rec 1: FILE_CONTENT = %\n",
            HexUtil::toHex(SFI_ENVIRONMENT_AND_HOLDER),
            calypsoCard->getFileBySfi(SFI_ENVIRONMENT_AND_HOLDER));

        mLogger->info("= #### End of the card processing\n");
    } break;
    case CardReaderEvent::CARD_INSERTED:
        mLogger->error(
            "CARD_INSERTED event: should not have occurred because of the "
            "MATCHED_ONLY selection mode chosen.\n");
        break;
    case CardReaderEvent::CARD_REMOVED:
        mLogger->trace(
            "There is no card inserted anymore. Return to the waiting "
            "state...\n");
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
        mReader->finalizeCardProcessing();
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
