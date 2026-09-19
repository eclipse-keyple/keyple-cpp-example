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

#include "ConfigurationUtil.hpp"

#include <memory>
#include <string>

#include "keyple/core/util/cpp/exception/IllegalStateException.hpp"
#include "keypop/reader/ConfigurableCardReader.hpp"

using keyple::core::util::cpp::exception::IllegalStateException;
using keypop::reader::ConfigurableCardReader;

const std::string ConfigurationUtil::ISO_CARD_PROTOCOL = "ISO_14443_4_CARD";
const std::string ConfigurationUtil::INNOVATRON_CARD_PROTOCOL
    = "INNOVATRON_B_PRIME_CARD";
const std::string ConfigurationUtil::SAM_PROTOCOL = "ISO_7816_3_T0";

const std::string ConfigurationUtil::CARD_READER_NAME_REGEX
    = ".*ASK LoGO.*|.*Contactless.*|.*ACR122U.*|.*00 01.*|.*5x21-CL 0.*";
const std::string ConfigurationUtil::SAM_READER_NAME_REGEX
    = ".*Identive.*|.*HID.*|.*00 00.*|.*5x21 0.*";

std::shared_ptr<CardReader>
ConfigurationUtil::getReader(
    std::shared_ptr<Plugin> plugin,
    const std::string& readerNameRegex,
    bool isContactless,
    const PcscReader::IsoProtocol& isoProtocol,
    const PcscReader::SharingMode sharingMode,
    const std::string& physicalProtocolName,
    const std::string& logicalProtocolName) {
    std::shared_ptr<CardReader> reader(plugin->findReader(readerNameRegex));

    if (reader == nullptr) {
        throw IllegalStateException(
            "No reader matching the regex '" + readerNameRegex + "' was found");
    }

    std::dynamic_pointer_cast<PcscReader>(
        plugin->getReaderExtension(typeid(PcscReader), reader->getName()))
        ->setContactless(isContactless)
        .setIsoProtocol(isoProtocol)
        .setSharingMode(sharingMode);

    std::dynamic_pointer_cast<ConfigurableCardReader>(reader)->activateProtocol(
        physicalProtocolName, logicalProtocolName);

    return reader;
}
