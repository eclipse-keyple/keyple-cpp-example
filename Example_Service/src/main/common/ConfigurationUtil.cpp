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

/* Keyple Core Util */
#include "IllegalStateException.h"

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

const std::string& ConfigurationUtil::getCardReaderName(std::shared_ptr<Plugin> plugin,
                                                        const std::string& readerNameRegex)
{
    for (const auto& readerName : plugin->getReaderNames()) {
        if (std::regex_match(readerName, std::regex(readerNameRegex))) {
            mLogger->info("Card reader, plugin; %, name: %\n", plugin->getName(), readerName);
            return readerName;
        }
    }

    throw IllegalStateException("Reader " +
                                readerNameRegex +
                                " not found in plugin " +
                                plugin->getName());
}
