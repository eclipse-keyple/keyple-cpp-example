/**************************************************************************************************
 * Copyright (c) 2023 Calypso Networks Association https://calypsonet.org/                        *
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
#include <sstream>

/* Keyple Card Calypso */
#include "CalypsoExtensionService.h"

/* Keyple Core Common */
#include "KeypleReaderExtension.h"

/* Keyple Core Service */
#include "ConfigurableReader.h"

/* Keyple Core Util */
#include "Exception.h"
#include "IllegalStateException.h"

/* Keyple Plugin Pcsc */
#include "PcscReader.h"
#include "PcscSupportedContactProtocol.h"
#include "PcscSupportedContactlessProtocol.h"

/* Keyple Service Resource */
#include "CardResourceServiceProvider.h"
#include "CardResourceProfileConfigurator.h"

using namespace keyple::card::calypso;
using namespace keyple::core::common;
using namespace keyple::core::service::resource;
using namespace keyple::core::util::cpp::exception;
using namespace keyple::plugin::pcsc;

/* READER CONFIGURATOR -------------------------------------------------------------------------- */

ConfigurationUtil::ReaderConfigurator::ReaderConfigurator() {}

void ConfigurationUtil::ReaderConfigurator::setupReader(std::shared_ptr<CardReader> cardReader)
{
    /* Configure the reader with parameters suitable for contactless operations */
    try {
        std::shared_ptr<KeypleReaderExtension> readerExtension =
            SmartCardServiceProvider::getService()
                ->getPlugin(cardReader)
                ->getReaderExtension(typeid(KeypleReaderExtension), cardReader->getName());
        auto pcscReader = std::dynamic_pointer_cast<PcscReader>(readerExtension);
        if (pcscReader) {
            pcscReader->setContactless(false)
                       .setIsoProtocol(PcscReader::IsoProtocol::ANY)
                       .setSharingMode(PcscReader::SharingMode::SHARED);
        }

    } catch (const Exception& e) {
        mLogger->error("Exception raised while setting up the reader %\n",
                       cardReader->getName(),
                       e);
    }
}

/* CONFIGURATION UTIL --------------------------------------------------------------------------- */

const std::string ConfigurationUtil::CARD_READER_NAME_REGEX =
    ".*ASK LoGO.*|.*Contactless.*|.*ACR122U.*|.*00 01.*|.*5x21-CL 0.*";
const std::string ConfigurationUtil::SAM_READER_NAME_REGEX =
    ".*Identive.*|.*HID.*|.*SAM.*|.*00 00.*|.*5x21 0.*";
const std::string ConfigurationUtil::SAM_PROTOCOL = "ISO_7816_3_T0";
const std::string ConfigurationUtil::ISO_CARD_PROTOCOL = "ISO_14443_4_CARD";
const std::string ConfigurationUtil::INNOVATRON_CARD_PROTOCOL = "INNOVATRON_B_PRIME_CARD";

const std::unique_ptr<Logger> ConfigurationUtil::mLogger =
    LoggerFactory::getLogger(typeid(ConfigurationUtil));

ConfigurationUtil::ConfigurationUtil() {}

const std::string ConfigurationUtil::getReaderName(std::shared_ptr<Plugin> plugin,
                                                   const std::string& readerNameRegex)
{
    std::string name = "";
    const std::regex nameRegex(readerNameRegex);

    for (const auto& readerName : plugin->getReaderNames()) {
        if (std::regex_match(readerName, nameRegex)) {
            mLogger->info("Card reader, plugin; %, name: %\n", plugin->getName(), readerName);
            name = readerName;
            return name;
        }
    }

    std::stringstream ss;
    ss << "Reader '" << readerNameRegex << "' not found in plugin '" << plugin->getName() << "'";
    throw IllegalStateException(ss.str());
}

std::shared_ptr<CardReader> ConfigurationUtil::getCardReader(std::shared_ptr<Plugin> plugin,
                                                             const std::string& readerNameRegex)
{
    /* Get the contactless reader whose name matches the provided regex */
    const std::string pcscContactlessReaderName = getReaderName(plugin, readerNameRegex);
    std::shared_ptr<CardReader> cardReader = plugin->getReader(pcscContactlessReaderName);

    /* Configure the reader with parameters suitable for contactless operations. */
    std::shared_ptr<PcscReader> reader =
        std::dynamic_pointer_cast<PcscReader>(
            plugin->getReaderExtension(typeid(PcscReader), pcscContactlessReaderName));
    reader->setContactless(true)
           .setIsoProtocol(PcscReader::IsoProtocol::T1)
           .setSharingMode(PcscReader::SharingMode::SHARED);

    std::dynamic_pointer_cast<ConfigurableCardReader>(cardReader)
        ->activateProtocol(PcscSupportedContactlessProtocol::ISO_14443_4.getName(),
                           ISO_CARD_PROTOCOL);

    return cardReader;
}

std::shared_ptr<CardReader> ConfigurationUtil::getSamReader(std::shared_ptr<Plugin> plugin,
                                                            const std::string& readerNameRegex)
{
    /* Get the contact reader dedicated for Calypso SAM whose name matches the provided regex */
    const std::string pcscContactReaderName = getReaderName(plugin, readerNameRegex);
    std::shared_ptr<CardReader> samReader = plugin->getReader(pcscContactReaderName);

    /* Configure the Calypso SAM reader with parameters suitable for contactless operations. */
    std::shared_ptr<PcscReader> reader =
        std::dynamic_pointer_cast<PcscReader>(
            plugin->getReaderExtension(typeid(PcscReader), pcscContactReaderName));
    reader->setContactless(false)
           .setIsoProtocol(PcscReader::IsoProtocol::ANY)
           .setSharingMode(PcscReader::SharingMode::SHARED);

    std::dynamic_pointer_cast<ConfigurableCardReader>(samReader)
        ->activateProtocol(PcscSupportedContactProtocol::ISO_7816_3_T0.getName(), SAM_PROTOCOL);

    return samReader;
}

std::shared_ptr<CalypsoSam> ConfigurationUtil::getSam(std::shared_ptr<CardReader> samReader)
{
    /* Create a SAM selection manager. */
    std::shared_ptr<CardSelectionManager> samSelectionManager =
        SmartCardServiceProvider::getService()->createCardSelectionManager();

    /* Create a SAM selection using the Calypso card extension. */
    samSelectionManager->prepareSelection(
        CalypsoExtensionService::getInstance()->createSamSelection());

    /* SAM communication: run the selection scenario. */
    std::shared_ptr<CardSelectionResult> samSelectionResult =
        samSelectionManager->processCardSelectionScenario(samReader);

    /* Check the selection result. */
    if (samSelectionResult->getActiveSmartCard() == nullptr) {
        throw IllegalStateException("The selection of the SAM failed.");
    }

    /* Get the Calypso SAM SmartCard resulting of the selection. */
    return std::dynamic_pointer_cast<CalypsoSam>(samSelectionResult->getActiveSmartCard());
}

void ConfigurationUtil::setupCardResourceService(std::shared_ptr<Plugin> plugin,
                                                 const std::string& readerNameRegex,
                                                 const std::string& samProfileName)
{
    /* Create a card resource extension expecting a SAM "C1" */
    std::shared_ptr<CalypsoSamSelection> samSelection =
        CalypsoExtensionService::getInstance()->createSamSelection();
    samSelection->filterByProductType(CalypsoSam::ProductType::SAM_C1);

    std::shared_ptr<CardResourceProfileExtension> samCardResourceExtension =
        CalypsoExtensionService::getInstance()->createSamResourceProfileExtension(samSelection);

    /* Get the service */
    std::shared_ptr<CardResourceService> cardResourceService =
        CardResourceServiceProvider::getService();

    /* Create a minimalist configuration (no plugin/reader observation) */
    cardResourceService
        ->getConfigurator()
        ->withPlugins(
            PluginsConfigurator::builder()
                ->addPlugin(plugin, std::shared_ptr<ReaderConfigurator>(new ReaderConfigurator()))
                 .build())
        .withCardResourceProfiles(
            {CardResourceProfileConfigurator::builder(samProfileName, samCardResourceExtension)
                ->withReaderNameRegex(readerNameRegex)
                 .build()})
        .configure();
    cardResourceService->start();

    /* Verify the resource availability */
    std::shared_ptr<CardResource> cardResource =
        cardResourceService->getCardResource(samProfileName);

    if (cardResource == nullptr) {
        std::stringstream ss;
        ss << "Unable to retrieve a SAM card resource for profile '"
           << samProfileName
           << "' from reader '"
           << readerNameRegex
           << "' in plugin '"
           << plugin->getName() << "'";
        throw IllegalStateException(ss.str());
    }

    /* Release the resource */
    cardResourceService->releaseCardResource(cardResource);
}