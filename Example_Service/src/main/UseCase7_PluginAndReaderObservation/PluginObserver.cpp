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

#include "PluginObserver.hpp"

#include <memory>
#include <string>
#include <vector>

#include "common/ConfigurationUtil.hpp"

#include "keyple/core/common/KeypleReaderExtension.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keyple/plugin/pcsc/PcscSupportedContactlessProtocol.hpp"
#include "keypop/reader/ConfigurableCardReader.hpp"
#include "keypop/reader/ObservableCardReader.hpp"

using keyple::core::common::KeypleReaderExtension;
using keyple::plugin::pcsc::PcscReader;
using keyple::plugin::pcsc::PcscSupportedContactlessProtocol;
using keypop::reader::ConfigurableCardReader;
using keypop::reader::ObservableCardReader;

PluginObserver::PluginObserver(
    const std::vector<std::shared_ptr<CardReader>>& initialReaders)
: mReaderObserver(std::make_shared<ReaderObserver>()) {
    for (const auto& reader : initialReaders) {
        if (std::dynamic_pointer_cast<ObservableCardReader>(reader)
            != nullptr) {
            addObserver(reader);
        }
    }
}

void
PluginObserver::onPluginEvent(const std::shared_ptr<PluginEvent> event) {
    for (const auto& readerName : event->getReaderNames()) {
        /* We retrieve the reader object from its name */
        std::shared_ptr<CardReader> reader
            = SmartCardServiceProvider::getService()
                  ->getPlugin(event->getPluginName())
                  ->getReader(readerName);

        mLogger->info(
            "PluginEvent: PLUGINNAME = %, READERNAME = %, Type = %\n",
            event->getPluginName(),
            readerName,
            event->getType());

        switch (event->getType()) {
        case PluginEvent::Type::READER_CONNECTED:
            /*
             * We are informed here of a connection of a reader. We add an
             * observer to this reader if this is possible.
             */
            mLogger->info("New reader! READERNAME = %\n", readerName);

            /*
             * Configure the reader with parameters suitable for contactless
             * operations.
             */
            setupReader(reader);

            if (std::dynamic_pointer_cast<ObservableCardReader>(reader)) {
                addObserver(reader);
            }
            break;

        case PluginEvent::Type::READER_DISCONNECTED:
            /*
             * We are informed here of a disconnection of a reader. The reader
             * object still exists but will be removed from the reader list
             * right after. Thus, we can properly remove the observer attached
             * to this reader before the list update.
             */
            mLogger->info("Reader removed. READERNAME = %\n", readerName);

            if (std::dynamic_pointer_cast<ObservableCardReader>(reader)) {
                mLogger->info(
                    "Clear observers of READERNAME = %\n", readerName);
                std::dynamic_pointer_cast<ObservableCardReader>(reader)
                    ->clearObservers();
            }
            break;

        default:
            mLogger->info(
                "Unexpected reader event. EVENT = %\n", event->getType());
            break;
        }
    }
}

void
PluginObserver::onPluginObservationError(
    const std::string& pluginName, const std::shared_ptr<Exception> e) {
    mLogger->error("An exception occurred in plugin '%': %\n", pluginName, e);
}

void
PluginObserver::setupReader(std::shared_ptr<CardReader> cardReader) {
    try {
        std::shared_ptr<KeypleReaderExtension> readerExtension
            = SmartCardServiceProvider::getService()
                  ->getPlugin(cardReader)
                  ->getReaderExtension(
                      typeid(KeypleReaderExtension), cardReader->getName());
        if (std::dynamic_pointer_cast<PcscReader>(readerExtension)) {
            std::dynamic_pointer_cast<PcscReader>(readerExtension)
                ->setContactless(true)
                .setIsoProtocol(PcscReader::IsoProtocol::T1)
                .setSharingMode(PcscReader::SharingMode::SHARED);
        }
    } catch (const Exception& e) {
        mLogger->error(
            "Exception raised while setting up the reader %\n",
            cardReader->getName(),
            e);
    }

    /* Activate the ISO14443 card protocol */
    auto configurable
        = std::dynamic_pointer_cast<ConfigurableCardReader>(cardReader);
    configurable->activateProtocol(
        PcscSupportedContactlessProtocol::ISO_14443_4.getName(),
        ConfigurationUtil::ISO_CARD_PROTOCOL);
}

void
PluginObserver::addObserver(std::shared_ptr<CardReader> reader) {
    mLogger->info("Add observer READERNAME = %\n", reader->getName());

    auto observer = std::dynamic_pointer_cast<ObservableCardReader>(reader);
    observer->setReaderObservationExceptionHandler(mReaderObserver);
    observer->addObserver(mReaderObserver);
    observer->startCardDetection(
        ObservableCardReader::DetectionMode::REPEATING);
}
