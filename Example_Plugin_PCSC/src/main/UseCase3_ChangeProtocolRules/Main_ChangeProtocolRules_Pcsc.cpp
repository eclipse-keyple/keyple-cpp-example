/******************************************************************************
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

#include "keyple/card/generic/GenericExtensionService.hpp"
#include "keyple/core/common/KeypleCardExtension.hpp"
#include "keyple/core/service/Plugin.hpp"
#include "keyple/core/service/SmartCardService.hpp"
#include "keyple/core/service/SmartCardServiceProvider.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keypop/reader/CardReader.hpp"
#include "keypop/reader/ConfigurableCardReader.hpp"
#include "keypop/reader/ReaderApiFactory.hpp"

using keyple::card::generic::GenericExtensionService;
using keyple::core::common::KeypleCardExtension;
using keyple::core::service::Plugin;
using keyple::core::service::SmartCardService;
using keyple::core::service::SmartCardServiceProvider;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keyple::plugin::pcsc::PcscPluginFactoryBuilder;
using keyple::plugin::pcsc::PcscReader;
using keypop::reader::CardReader;
using keypop::reader::ConfigurableCardReader;
using keypop::reader::ReaderApiFactory;

/**
 * <h1>Use Case PC/SC 3 – Change of a protocol identification rule (PC/SC)</h1>
 *
 * <p>Here we demonstrate how to add a protocol rule to target a specific card
 * technology by applying a regular expression on the ATR provided by the
 * reader.
 *
 * <p>This feature of the PC/SC plugin is useful for extending the set of rules
 * already supported, but also for solving compatibility issues with some
 * readers producing ATRs that do not work with the built-in rules.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Configure the plugin to add a new protocol rule targeting Mifare
 *       Classic 4K cards.
 *   <li>Attempts to select a Mifare Classic 4K card with a protocol based
 *       selection.
 *   <li>Display the selection result.
 * </ul>
 *
 * In a real application, these regular expressions must be customized to the
 * names of the devices used.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_ChangeProtocolRules_Pcsc { };
static const std::unique_ptr<Logger> logger
    = LoggerFactory::getLogger(typeid(Main_ChangeProtocolRules_Pcsc));

static const std::string READER_PROTOCOL_MIFARE_CLASSIC_4_K
    = "MIFARE_CLASSIC_4K";
static const std::string CARD_PROTOCOL_MIFARE_CLASSIC_4_K = "MIFARE_CLASSIC_4K";

static int
runExample() {
    /* Get the instance of the SmartCardService (singleton pattern) */
    auto smartCardService(SmartCardServiceProvider::getService());

    /*
     * Register the PcscPlugin with the SmartCardService, set the two regular
     * expression matching the expected devices, get the corresponding generic
     * plugin in return.
     */
    auto factory(PcscPluginFactoryBuilder::builder()
                     ->updateProtocolIdentificationRule(
                         READER_PROTOCOL_MIFARE_CLASSIC_4_K,
                         "3B8F8001804F0CA0000003060300020000000069")
                     .build());
    auto plugin(smartCardService->registerPlugin(factory));

    /*
     * Get the first available reader (we assume that a single contactless
     * reader is connected).
     */
    std::shared_ptr<CardReader> reader = plugin->getReaders()[1];

    std::dynamic_pointer_cast<ConfigurableCardReader>(reader)->activateProtocol(
        READER_PROTOCOL_MIFARE_CLASSIC_4_K, CARD_PROTOCOL_MIFARE_CLASSIC_4_K);

    /* Configure the reader for contactless operations */
    std::dynamic_pointer_cast<PcscReader>(
        plugin->getReaderExtension(typeid(PcscReader), reader->getName()))
        ->setContactless(true)
        .setIsoProtocol(PcscReader::IsoProtocol::T1)
        .setSharingMode(PcscReader::SharingMode::SHARED);

    /* Get the generic card extension service */
    auto cardExtension = GenericExtensionService::getInstance();

    /*
     * Verify that the extension's API level is consistent with the current
     * service.
     */
    smartCardService->checkCardExtension(cardExtension);

    /* Check if a card is present in the reader */
    if (!reader->isCardPresent()) {
        logger->error("No card is present in the reader\n");
        return -1;
    }

    /* Retrieve the reader API factory. */
    std::shared_ptr<ReaderApiFactory> readerApiFactory(
        smartCardService->getReaderApiFactory());

    /* Get the core card selection manager */
    auto cardSelectionManager(readerApiFactory->createCardSelectionManager());

    /* Create a generic card selection with a MIFARE CLASSIC protocol filter. */
    auto cardSelector(readerApiFactory->createBasicCardSelector());
    cardSelector->filterByCardProtocol(CARD_PROTOCOL_MIFARE_CLASSIC_4_K);

    /*
     * Prepare the selection by adding the created generic selection to the card
     * selection scenario.
     */
    cardSelectionManager->prepareSelection(
        cardSelector, cardExtension->createGenericCardSelectionExtension());

    /* Actual card communication: run the selection scenario */
    const auto selectionResult
        = cardSelectionManager->processCardSelectionScenario(reader);

    /* Check the selection result */
    if (selectionResult->getActiveSmartCard() == nullptr) {
        logger->error("The selection of the card failed\n");
        return -1;
    }

    /* Get the SmartCard resulting of the selection */
    const auto smartCard = selectionResult->getActiveSmartCard();

    logger->info("= SmartCard = %\n", smartCard);

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
