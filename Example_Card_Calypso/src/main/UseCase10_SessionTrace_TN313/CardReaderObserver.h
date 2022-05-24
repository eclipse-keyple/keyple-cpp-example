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

/* Calypsonet Terminal Calypso */
#include "CardSecuritySetting.h"

/* Calypsonet Terminal Reader */
#include "CardReader.h"
#include "CardReaderObserverSpi.h"
#include "CardReaderObservationExceptionHandlerSpi.h"
#include "CardSelectionManager.h"

/* Keyple Core Util */
#include "ByteArrayUtil.h"
#include "LoggerFactory.h"

using namespace calypsonet::terminal::calypso::transaction;
using namespace calypsonet::terminal::reader;
using namespace calypsonet::terminal::reader::selection;
using namespace calypsonet::terminal::reader::spi;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp;

/**
 * (package-private)
 *
 * <p>A reader Observer handles card event such as CARD_INSERTED, CARD_MATCHED, CARD_REMOVED
 */
class CardReaderObserver final
: public CardReaderObserverSpi, public CardReaderObservationExceptionHandlerSpi {
public:
    /**
     * (package-private)<br>
     * Constructor.
     *
     * @param cardReader The card reader.
     * @param cardSelectionManager The card selection manager.
     * @param cardSecuritySetting The card security settings.
     */
    CardReaderObserver(std::shared_ptr<CardReader> cardReader,
                       std::shared_ptr<CardSelectionManager> cardSelectionManager,
                       std::shared_ptr<CardSecuritySetting> cardSecuritySetting);

    /**
     * {@inheritDoc}
     */
    void onReaderEvent(const std::shared_ptr<CardReaderEvent> event) override;

    /**
     * {@inheritDoc}
     */
    void onReaderObservationError(const std::string& pluginName,
                                  const std::string& readerName,
                                  const std::shared_ptr<Exception> e) override;

private:
    /**
     *
     */
    const std::unique_ptr<Logger> mLogger = LoggerFactory::getLogger(typeid(CardReaderObserver));

    /**
     *
     */
    std::shared_ptr<CardReader> mCardReader;

    /**
     *
     */
    std::shared_ptr<CardSecuritySetting> mCardSecuritySetting;

    /**
     *
     */
    std::shared_ptr<CardSelectionManager> mCardSelectionManager;

    /**
     *
     */
    const std::vector<uint8_t> mNewEventRecord =
        ByteArrayUtil::fromHex("8013C8EC55667788112233445566778811223344556677881122334455");
};
