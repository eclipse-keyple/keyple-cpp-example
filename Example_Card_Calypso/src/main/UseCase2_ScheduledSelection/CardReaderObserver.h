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

/* Calypsonet Terminal Reader */
#include "CardReader.h"
#include "CardReaderObserverSpi.h"
#include "CardReaderObservationExceptionHandlerSpi.h"
#include "CardSelectionManager.h"

/* Keyple Core Util */
#include "LoggerFactory.h"

using namespace calypsonet::terminal::reader;
using namespace calypsonet::terminal::reader::selection;
using namespace calypsonet::terminal::reader::spi;
using namespace keyple::core::util::cpp;

/**
 * A reader Observer handles card event such as CARD_INSERTED, CARD_MATCHED, CARD_REMOVED
 */
class CardReaderObserver final
: public CardReaderObserverSpi, public CardReaderObservationExceptionHandlerSpi {
public:
    /**
     * (package-private)<br>
     * Constructor.
     *
     * <p>Note: the reader is provided here for convenience but could also be retrieved from the
     * {@link SmartCardService} with its name and that of the plugin both present in the {@link
     * CardReaderEvent}.
     *
     * @param reader The card reader.
     * @param cardSelectionManager The card selection manager.
     */
    CardReaderObserver(std::shared_ptr<CardReader> reader,
                       std::shared_ptr<CardSelectionManager> cardSelectionManager);

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
    std::shared_ptr<CardReader> mReader;

    /**
     *
     */
    std::shared_ptr<CardSelectionManager> mCardSelectionManager;
};
