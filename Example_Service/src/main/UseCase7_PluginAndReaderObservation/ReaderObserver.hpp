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

#pragma once

#include <memory>
#include <string>

#include "keyple/core/service/SmartCardService.hpp"
#include "keyple/core/service/SmartCardServiceProvider.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keypop/reader/CardReaderEvent.hpp"
#include "keypop/reader/spi/CardReaderObservationExceptionHandlerSpi.hpp"
#include "keypop/reader/spi/CardReaderObserverSpi.hpp"

using keyple::core::service::SmartCardService;
using keyple::core::service::SmartCardServiceProvider;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keypop::reader::CardReaderEvent;
using keypop::reader::spi::CardReaderObservationExceptionHandlerSpi;
using keypop::reader::spi::CardReaderObserverSpi;

/**
 * Implements the reader observation SPIs.<br>
 * A reader Observer to handle card events such as CARD_INSERTED, CARD_MATCHED,
 * CARD_REMOVED
 *
 * @since 2.0.0
 */
class ReaderObserver : public CardReaderObserverSpi,
                       public CardReaderObservationExceptionHandlerSpi {
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
    void onReaderObservationError(
        const std::string& pluginName,
        const std::string& readerName,
        const std::shared_ptr<std::exception> e) override;

private:
    /**
     *
     */
    const std::unique_ptr<Logger> mLogger
        = LoggerFactory::getLogger(typeid(ReaderObserver));

    /**
     *
     */
    const std::shared_ptr<SmartCardService> smartCardService
        = SmartCardServiceProvider::getService();
};
