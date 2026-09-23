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

#include "common/ConfigurationUtil.hpp"

#include <string>

const std::string ConfigurationUtil::AID_EMV_PPSE
    = "325041592E5359532E4444463031";
const std::string ConfigurationUtil::AID_KEYPLE_PREFIX = "315449432E";
const std::string ConfigurationUtil::ISO_CARD_PROTOCOL = "ISO_14443_4_CARD";
const std::string ConfigurationUtil::CONTACTLESS_READER_NAME_REGEX
    = ".*ASK LoGO.*|.*Contactless.*|.*ACR122U.*|.*00 01.*|.*5x21-CL 0.*";
const std::string ConfigurationUtil::CONTACT_READER_NAME_REGEX
    = ".*Identive.*|.*HID.*|.*00 00.*|.*5x21 0.*";

const std::unique_ptr<Logger> ConfigurationUtil::mLogger
    = LoggerFactory::getLogger(typeid(ConfigurationUtil));
