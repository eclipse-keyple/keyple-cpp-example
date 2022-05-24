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

#include "ConfigurationUtil.h"

#include <regex>

/* Keyple Card Calypso */
#include "CalypsoExtensionService.h"

/* Keyple Core Common */
#include "KeypleReaderExtension.h"

/* Keyple Core Service */
#include "ConfigurableReader.h"

/* Keyple Core Util */
#include "ContactlessCardCommonProtocol.h"
#include "Exception.h"
#include "IllegalStateException.h"
#include "StringUtils.h"

/* Keyple Plugin Pcsc */
#include "PcscReader.h"
#include "PcscSupportedContactlessProtocol.h"

/* Keyple Service Resource */
#include "CardResourceServiceProvider.h"
#include "CardResourceProfileConfigurator.h"

using namespace keyple::card::calypso;
using namespace keyple::core::common;
using namespace keyple::core::service::resource;
using namespace keyple::core::util::cpp::exception;
using namespace keyple::core::util::protocol;
using namespace keyple::plugin::pcsc;

/* READER CONFIGURATOR -------------------------------------------------------------------------- */

ConfigurationUtil::ReaderConfigurator::ReaderConfigurator() {}

void ConfigurationUtil::ReaderConfigurator::setupReader(std::shared_ptr<Reader> reader)
{
    /* Configure the reader with parameters suitable for contactless operations */
    try {
        std::shared_ptr<KeypleReaderExtension> readerExtension =
            reader->getExtension(typeid(KeypleReaderExtension));
        auto pcscReader = std::dynamic_pointer_cast<PcscReader>(readerExtension);
        if (pcscReader) {
            pcscReader->setContactless(false)
                       .setIsoProtocol(PcscReader::IsoProtocol::T0)
                       .setSharingMode(PcscReader::SharingMode::SHARED);
        }
    } catch (const Exception& e) {
        mLogger->error("Exception raised while setting up the reader %\n", reader->getName(), e);
    }
}

/* CONFIGURATION UTIL --------------------------------------------------------------------------- */

const std::string ConfigurationUtil::CARD_READER_NAME_REGEX = ".*ASK LoGO.*|.*Contactless.*|.*00 01.*";
const std::string ConfigurationUtil::SAM_READER_NAME_REGEX = ".*Identive.*|.*HID.*|.*00 00.*";
const std::unique_ptr<Logger> ConfigurationUtil::mLogger =
    LoggerFactory::getLogger(typeid(ConfigurationUtil));

ConfigurationUtil::ConfigurationUtil() {}

std::shared_ptr<Reader> ConfigurationUtil::getCardReader(std::shared_ptr<Plugin> plugin,
                                                         const std::string& readerNameRegex)
{
    const std::regex nameRegex(readerNameRegex);

    for (const auto& readerName : plugin->getReaderNames()) {
        if (std::regex_match(readerName, nameRegex)) {
            auto reader = std::dynamic_pointer_cast<ConfigurableReader>(plugin->getReader(readerName));

            /* Configure the reader with parameters suitable for contactless operations */
            auto pcscReader =
                std::dynamic_pointer_cast<PcscReader>(reader->getExtension(typeid(PcscReader)));

            pcscReader->setContactless(true)
                       .setIsoProtocol(PcscReader::IsoProtocol::T1)
                       .setSharingMode(PcscReader::SharingMode::SHARED);

            reader->activateProtocol(PcscSupportedContactlessProtocol::ISO_14443_4.getName(),
                                     ContactlessCardCommonProtocol::ISO_14443_4.getName());

            mLogger->info("Card reader, plugin; %, name: %\n",
                          plugin->getName(),
                          reader->getName());

            return reader;
        }
    }

    throw IllegalStateException(
              StringUtils::format("Reader '%s' not found in plugin '%s'",
                                  readerNameRegex,
                                  plugin->getName()));
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
        throw IllegalStateException(
                  StringUtils::format("Unable to retrieve a SAM card resource for profile '%s' " \
                                      "from reader '%s' in plugin '%s'",
                                      samProfileName,
                                      readerNameRegex,
                                      plugin->getName()));
    }

    /* Release the resource */
    cardResourceService->releaseCardResource(cardResource);
}