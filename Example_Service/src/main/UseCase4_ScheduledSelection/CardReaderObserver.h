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
#include "CardReaderObservationExceptionHandlerSpi.h"
#include "CardReaderObserverSpi.h"
#include "CardSelectionManager.h"

/* Keyple Core Util */
#include "LoggerFactory.h"

/* Keyple Core Service */
#include "Reader.h"

using namespace calypsonet::terminal::reader::selection;
using namespace calypsonet::terminal::reader::spi;
using namespace keyple::core::service;
using namespace keyple::core::util::cpp;

/**
 * (package-private)<br>
 * Implements the reader observation SPIs.<br>
 * A reader Observer to handle card events such as CARD_INSERTED, CARD_MATCHED, CARD_REMOVED
 *
 * @since 2.0.0.0
 */
class CardReaderObserver :
public CardReaderObserverSpi,
public CardReaderObservationExceptionHandlerSpi {
public:
    /**
     * 
     */
    CardReaderObserver(std::shared_ptr<Reader> reader, 
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
    std::shared_ptr<Reader> mReader;
    
    /**
     * 
     */
    std::shared_ptr<CardSelectionManager> mCardSelectionManager;
};
