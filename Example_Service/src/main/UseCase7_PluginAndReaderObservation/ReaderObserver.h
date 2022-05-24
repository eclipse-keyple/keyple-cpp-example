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
#include "CardReaderEvent.h"
#include "CardReaderObservationExceptionHandlerSpi.h"
#include "CardReaderObserverSpi.h"

/* Keyple Core Util */
#include "LoggerFactory.h"

using namespace calypsonet::terminal::reader;
using namespace calypsonet::terminal::reader::spi;
using namespace keyple::core::util::cpp;

/**
 * (package-private)<br>
 * Implements the reader observation SPIs.<br>
 * A reader Observer to handle card events such as CARD_INSERTED, CARD_MATCHED, CARD_REMOVED
 *
 * @since 2.0.0
 */
class ReaderObserver
: public CardReaderObserverSpi, public CardReaderObservationExceptionHandlerSpi {
public:
    /**
     * 
     */
    virtual ~ReaderObserver() = default;
    
    /**
     *
     */
    void onReaderEvent(const std::shared_ptr<CardReaderEvent> event) override;


    /**
     *
     */
    void onReaderObservationError(const std::string& pluginName,
                                  const std::string& readerName,
                                  const std::shared_ptr<Exception> e) override;

private:
    /**
     *
     */
    const std::unique_ptr<Logger> mLogger = LoggerFactory::getLogger(typeid(ReaderObserver));
};
