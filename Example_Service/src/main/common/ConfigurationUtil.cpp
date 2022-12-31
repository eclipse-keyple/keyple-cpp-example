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

#include "ConfigurationUtil.h"

#include <regex>

/* Keyple Core Service */
#include "ConfigurableReader.h"

/* Keyple Core Util */
#include "IllegalStateException.h"

/* Keyple Plugin Pcsc */
#include "PcscReader.h"
#include "PcscSupportedContactlessProtocol.h"

using namespace keyple::plugin::pcsc;
using namespace keyple::core::service;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp::exception;

const std::string ConfigurationUtil::AID_EMV_PPSE = "325041592E5359532E4444463031";
const std::string ConfigurationUtil::AID_KEYPLE_PREFIX = "315449432E";
const std::string ConfigurationUtil::ISO_CARD_PROTOCOL = "ISO_14443_4_CARD";
const std::string ConfigurationUtil::CONTACTLESS_READER_NAME_REGEX =
    ".*ASK LoGO.*|.*Contactless.*|.*ACR122U.*|.*00 01.*|.*5x21-CL 0.*";
const std::string ConfigurationUtil::CONTACT_READER_NAME_REGEX =
    ".*Identive.*|.*HID.*|.*00 00.*|.*5x21 0.*";

const std::unique_ptr<Logger> ConfigurationUtil::mLogger =
    LoggerFactory::getLogger(typeid(ConfigurationUtil));

std::shared_ptr<Reader> ConfigurationUtil::getCardReader(std::shared_ptr<Plugin> plugin,
                                                         const std::string& readerNameRegex)
{
    for (const auto& readerName : plugin->getReaderNames()) {
        if (std::regex_match(readerName, std::regex(readerNameRegex))) {
            auto reader = std::dynamic_pointer_cast<ConfigurableReader>(plugin->getReader(readerName));

            /* Configure the reader with parameters suitable for contactless operations */
            std::shared_ptr<KeypleReaderExtension> ext = reader->getExtension(typeid(PcscReader));
            auto pcscReader = std::dynamic_pointer_cast<PcscReader>(ext);
            pcscReader->setContactless(true)
                       .setIsoProtocol(PcscReader::IsoProtocol::T1)
                       .setSharingMode(PcscReader::SharingMode::SHARED);
            reader->activateProtocol(PcscSupportedContactlessProtocol::ISO_14443_4.getName(),
                                     ISO_CARD_PROTOCOL);

            mLogger->info("Card reader, plugin; %, name: %\n", plugin->getName(), reader->getName());

            return reader;
        }
    }

    throw IllegalStateException("Reader " +
                                readerNameRegex +
                                " not found in plugin " +
                                plugin->getName());
}
