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

#include <exception>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "keyple/card/calypso/crypto/legacysam/LegacySamExtensionService.hpp"
#include "keyple/card/calypso/crypto/legacysam/LegacySamUtil.hpp"
#include "keyple/core/service/Plugin.hpp"
#include "keyple/core/service/SmartCardService.hpp"
#include "keyple/core/service/SmartCardServiceProvider.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keyple/core/util/cpp/exception/IllegalStateException.hpp"
#include "keyple/plugin/pcsc/PcscCardCommunicationProtocol.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keypop/calypso/crypto/legacysam/LegacySamApiFactory.hpp"
#include "keypop/calypso/crypto/legacysam/sam/LegacySam.hpp"
#include "keypop/calypso/crypto/legacysam/transaction/FreeTransactionManager.hpp"
#include "keypop/reader/CardReader.hpp"
#include "keypop/reader/ChannelControl.hpp"
#include "keypop/reader/ReaderApiFactory.hpp"
#include "keypop/reader/selection/CardSelectionManager.hpp"
#include "keypop/reader/selection/CardSelectionResult.hpp"
#include "keypop/reader/selection/IsoCardSelector.hpp"

#include "../common/ConfigurationUtil.hpp"

using keyple::card::calypso::crypto::legacysam::LegacySamExtensionService;
using keyple::card::calypso::crypto::legacysam::LegacySamUtil;
using keyple::core::service::Plugin;
using keyple::core::service::SmartCardService;
using keyple::core::service::SmartCardServiceProvider;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keyple::core::util::cpp::exception::IllegalStateException;
using keyple::plugin::pcsc::PcscCardCommunicationProtocol;
using keyple::plugin::pcsc::PcscPluginFactoryBuilder;
using keyple::plugin::pcsc::PcscReader;
using keypop::calypso::crypto::legacysam::LegacySamApiFactory;
using keypop::calypso::crypto::legacysam::sam::LegacySam;
using keypop::calypso::crypto::legacysam::transaction::FreeTransactionManager;
using keypop::reader::CardReader;
using keypop::reader::ChannelControl;
using keypop::reader::ReaderApiFactory;
using keypop::reader::selection::CardSelectionManager;
using keypop::reader::selection::CardSelectionResult;
using keypop::reader::selection::IsoCardSelector;

/**
 * Handles the reading of the counters and ceilings of a legacy SAM (PC/SC)
 * using the Legacy SAM extension service.
 *
 * <p>This class demonstrates how to set up a simple Legacy SAM transaction to
 * read counters and ceilings.
 *
 * <p>Operations and results are systematically logged, facilitating
 * comprehensive monitoring, tracking, and debugging. In the occurrence of
 * unexpected behaviors or anomalies, runtime exceptions are generated,
 * offering clear insights into issues for prompt resolution.
 */
class Main_ReadLegacySamCountersAndCeilings_Pcsc { };
static std::unique_ptr<Logger> logger = LoggerFactory::getLogger(
    typeid(Main_ReadLegacySamCountersAndCeilings_Pcsc));

/* The plugin used to manage the readers. */
static std::shared_ptr<Plugin> plugin;
/* The reader used to communicate with the SAM. */
static std::shared_ptr<CardReader> samReader;
/* The factory used to create the selection manager and card selectors. */
static std::shared_ptr<ReaderApiFactory> readerApiFactory;
/* The Legacy SAM factory used to create the transaction managers. */
static std::shared_ptr<LegacySamApiFactory> legacySamApiFactory;

/**
 * Formats a counter/ceiling map as a human-readable, pretty-printed JSON-like
 * string.
 */
static std::string
countersToString(const std::map<const int, int>& counters) {
    std::ostringstream oss;
    oss << "{\n";
    bool first = true;
    for (const auto& entry : counters) {
        if (!first) {
            oss << ",\n";
        }
        first = false;
        oss << "  \"" << entry.first << "\": " << entry.second;
    }
    oss << "\n}";

    return oss.str();
}

/**
 * Initializes the Keyple service.
 *
 * <p>Gets an instance of the smart card service, registers the PC/SC plugin,
 * and prepares the reader API factory for use.
 */
static void
initKeypleService() {
    std::shared_ptr<SmartCardService> smartCardService(
        SmartCardServiceProvider::getService());
    plugin = smartCardService->registerPlugin(
        PcscPluginFactoryBuilder::builder()->build());
    readerApiFactory = smartCardService->getReaderApiFactory();
}

/**
 * Initializes the SAM reader with specific configurations.
 */
static void
initSamReader() {
    samReader = ConfigurationUtil::getReader(
        plugin,
        ConfigurationUtil::SAM_READER_NAME_REGEX,
        false,
        PcscReader::IsoProtocol::ANY,
        PcscReader::SharingMode::SHARED,
        PcscCardCommunicationProtocol::ISO_7816_3.getName(),
        ConfigurationUtil::SAM_PROTOCOL);
}

/**
 * Initializes the Legacy SAM extension service.
 */
static void
initLegacySamExtensionService() {
    std::shared_ptr<LegacySamExtensionService> legacySamExtensionService(
        LegacySamExtensionService::getInstance());
    SmartCardServiceProvider::getService()->checkCardExtension(
        legacySamExtensionService);
    legacySamApiFactory = legacySamExtensionService->getLegacySamApiFactory();
}

/**
 * Selects the SAM C1 for the transaction.
 */
static std::shared_ptr<LegacySam>
selectSam(std::shared_ptr<CardReader> reader) {
    std::shared_ptr<CardSelectionManager> samSelectionManager(
        readerApiFactory->createCardSelectionManager());

    std::shared_ptr<IsoCardSelector> cardSelector(
        readerApiFactory->createIsoCardSelector());
    cardSelector->filterByPowerOnData(
        LegacySamUtil::buildPowerOnDataFilter(
            LegacySam::ProductType::SAM_C1, ""));

    samSelectionManager->prepareSelection(
        cardSelector, legacySamApiFactory->createLegacySamSelectionExtension());

    const std::shared_ptr<CardSelectionResult> samSelectionResult(
        samSelectionManager->processCardSelectionScenario(reader));

    if (samSelectionResult->getActiveSmartCard() == nullptr) {
        throw IllegalStateException("The selection of the SAM failed.");
    }

    return std::dynamic_pointer_cast<LegacySam>(
        samSelectionResult->getActiveSmartCard());
}

static int
runExample() {
    /* Initialize the context */
    initKeypleService();
    initLegacySamExtensionService();
    initSamReader();

    /* Get the Calypso legacy SAM SmartCard resulting of the selection */
    std::shared_ptr<LegacySam> sam(selectSam(samReader));

    /* Create a transaction manager */
    std::shared_ptr<FreeTransactionManager> samTransactionManager(
        legacySamApiFactory->createFreeTransactionManager(samReader, sam));

    /* Process the transaction to read counters and ceilings */
    samTransactionManager->prepareReadAllCountersStatus().processCommands(
        ChannelControl::KEEP_OPEN);

    /* Output results */
    logger->info(
        "\nSAM event counters =\n%\n", countersToString(sam->getCounters()));
    logger->info(
        "\nSAM event ceilings =\n%\n",
        countersToString(sam->getCounterCeilings()));

    return 0;
}

int
main() {
    try {
        return runExample();

    } catch (const std::exception& e) {
        logger->error("Example terminated on exception: %\n", e.what());
        return 1;
    }
}
