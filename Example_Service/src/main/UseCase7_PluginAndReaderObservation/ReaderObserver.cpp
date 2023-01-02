/**************************************************************************************************
 * Copyright (c) 2022 Calypso Networks Association https://calypsonet.org/                        *
 *                                                                                                *
 * See the NOTICE file(s) distributed with this work for additional information regarding         *
 * copyright ownership.                                                                           *
 *                                                                                                *
 * This program and the accompanying materials are made available under the terms of the Eclipse  *
 * Public License 2.0 which is available at http://www.eclipse.org/legal/epl-2.0                  *
 *                                                                                                *
 * SPDX-License-Identifier: EPL-2.0                                                               *
 **************************************************************************************************/

#include "ReaderObserver.h"

/* Keyple Core Service */
#include "SmartCardServiceProvider.h"

using namespace keyple::core::service;

void ReaderObserver::onReaderEvent(const std::shared_ptr<CardReaderEvent> event)
{
    /* Just log the event */
    const std::string pluginName =
        smartCardService->getPlugin(smartCardService->getReader(event->getReaderName()))
            ->getName();
    mLogger->info("Event: PLUGINNAME = %, READERNAME = %, EVENT = %\n",
                  pluginName,
                  event->getReaderName(),
                  event->getType());

    if (event->getType() != CardReaderEvent::Type::CARD_REMOVED) {
        std::dynamic_pointer_cast<ObservableCardReader>(
            smartCardService->getPlugin(pluginName)->getReader(event->getReaderName()))
                ->finalizeCardProcessing();
    }
}

void ReaderObserver::onReaderObservationError(const std::string& pluginName,
                                              const std::string& readerName,
                                              const std::shared_ptr<Exception> e)
{
    mLogger->error("An exception occurred in plugin '%', reader '%': %\n",
                   pluginName,
                   readerName,
                   e);
}
