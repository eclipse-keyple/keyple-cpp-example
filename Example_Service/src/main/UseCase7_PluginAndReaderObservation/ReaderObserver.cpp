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

#include "ReaderObserver.h"

/* Keyple Core Service */
#include "ObservableReader.h"
#include "ReaderEvent.h"
#include "SmartCardServiceProvider.h"

using namespace keyple::core::service;

void ReaderObserver::onReaderEvent(const std::shared_ptr<CardReaderEvent> event)
{
    /* Just log the event */
    mLogger->info("Event: PLUGINNAME = %, READERNAME = %, EVENT = %\n",
                  std::dynamic_pointer_cast<ReaderEvent>(event)->getPluginName(),
                  event->getReaderName(),
                  event->getType());

    if (event->getType() != CardReaderEvent::Type::CARD_REMOVED) {
        auto readerEvent = std::dynamic_pointer_cast<ReaderEvent>(event);
        std::shared_ptr<Reader> reader =
            SmartCardServiceProvider::getService()->getPlugin(readerEvent->getPluginName())
                                                  ->getReader(event->getReaderName());
        std::dynamic_pointer_cast<ObservableReader>(reader)->finalizeCardProcessing();
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
