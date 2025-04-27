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

#include "ReaderObserver.hpp"

#include "keypop/reader/ObservableCardReader.hpp"

using keypop::reader::ObservableCardReader;

void
ReaderObserver::onReaderEvent(const std::shared_ptr<CardReaderEvent> event) {
    /* Just log the event */
    const std::string pluginName
        = smartCardService
              ->getPlugin(smartCardService->getReader(event->getReaderName()))
              ->getName();
    mLogger->info(
        "Event: PLUGINNAME = %, READERNAME = %, EVENT = %\n",
        pluginName,
        event->getReaderName(),
        event->getType());

    if (event->getType() != CardReaderEvent::Type::CARD_REMOVED) {
        std::dynamic_pointer_cast<ObservableCardReader>(
            smartCardService->getPlugin(pluginName)
                ->getReader(event->getReaderName()))
            ->finalizeCardProcessing();
    }
}

void
ReaderObserver::onReaderObservationError(
    const std::string& pluginName,
    const std::string& readerName,
    const std::shared_ptr<std::exception> e) {
    mLogger->error(
        "An exception occurred in plugin '%', reader '%': %\n",
        pluginName,
        readerName,
        e);
}
