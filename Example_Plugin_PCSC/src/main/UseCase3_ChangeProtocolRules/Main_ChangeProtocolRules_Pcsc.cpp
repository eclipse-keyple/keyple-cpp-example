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

/* Keyple Core Util */
#include "LoggerFactory.h"

/* Keyple Core Service */
#include "ConfigurableReader.h"
#include "SmartCardService.h"
#include "SmartCardServiceProvider.h"

/* Keyple Plugin Pcsc */
#include "PcscPluginFactoryBuilder.h"
#include "PcscReader.h"

/* Keyple Card Generic */
#include "GenericExtensionService.h"

/* Keyple Core Common */
#include "KeypleCardExtension.h"

using namespace keyple::card::generic;
using namespace keyple::core::common;
using namespace keyple::core::util::cpp;
using namespace keyple::core::service;
using namespace keyple::plugin::pcsc;

/**
 *
 *
 * <h1>Use Case PC/SC 3 – Change of a protocol identification rule (PC/SC)</h1>
 *
 * <p>Here we demonstrate how to add a protocol rule to target a specific card technology by
 * applying a regular expression on the ATR provided by the reader.
 *
 * <p>This feature of the PC/SC plugin is useful for extending the set of rules already supported,
 * but also for solving compatibility issues with some readers producing ATRs that do not work with
 * the built-in rules.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Configure the plugin to add a new protocol rule targeting Mifare Classic 4K cards.
 *   <li>Attempts to select a Mifare Classic 4K card with a protocol based selection.
 *   <li>Display the selection result.
 * </ul>
 *
 * In a real application, these regular expressions must be customized to the names of the devices
 * used.
 *
 * <p>All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_ChangeProtocolRules_Pcsc {};
static const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_ChangeProtocolRules_Pcsc));

static const std::string READER_PROTOCOL_MIFARE_CLASSIC_4_K = "MIFARE_CLASSIC_4K";
static const std::string CARD_PROTOCOL_MIFARE_CLASSIC_4_K = "MIFARE_CLASSIC_4K";

int main()
{
    /* Get the instance of the SmartCardService (singleton pattern) */
    std::shared_ptr<SmartCardService> smartCardService = SmartCardServiceProvider::getService();

    /*
     * Register the PcscPlugin with the SmartCardService, set the two regular expression matching
     * the expected devices, get the corresponding generic plugin in return.
     */
    std::shared_ptr<Plugin> plugin =
        smartCardService->registerPlugin(
            PcscPluginFactoryBuilder::builder()
                ->updateProtocolIdentificationRule(
                    READER_PROTOCOL_MIFARE_CLASSIC_4_K, "3B8F8001804F0CA0000003060300020000000069")
                .build());

    /* Get the first available reader (we assume that a single contactless reader is connected) */
    std::shared_ptr<Reader> reader = plugin->getReaders()[1];

    std::dynamic_pointer_cast<ConfigurableReader>(reader)
        ->activateProtocol(READER_PROTOCOL_MIFARE_CLASSIC_4_K, CARD_PROTOCOL_MIFARE_CLASSIC_4_K);

    /* Configure the reader for contactless operations */
    std::dynamic_pointer_cast<PcscReader>(reader->getExtension(typeid(PcscReader)))
        ->setContactless(true)
         .setIsoProtocol(PcscReader::IsoProtocol::T1)
         .setSharingMode(PcscReader::SharingMode::SHARED);

    /* Get the generic card extension service */
    std::shared_ptr<GenericExtensionService> cardExtension = GenericExtensionService::getInstance();

    /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(cardExtension);

    /* Check if a card is present in the reader */
    if (!reader->isCardPresent()) {
        logger->error("No card is present in the reader\n");
        return -1;
    }

    /* Get the core card selection manager */
    std::unique_ptr<CardSelectionManager> cardSelectionManager =
        smartCardService->createCardSelectionManager();

    /*
     * Create a card selection using the generic card extension without specifying any filter
     * (protocol/ATR/DFName).
     */
    std::shared_ptr<GenericCardSelection> cardSelection = cardExtension->createCardSelection();
    cardSelection->filterByCardProtocol(CARD_PROTOCOL_MIFARE_CLASSIC_4_K);

    /*
     * Prepare the selection by adding the created generic selection to the card selection scenario.
     */
    cardSelectionManager->prepareSelection(cardSelection);

    /* Actual card communication: run the selection scenario */
    const std::shared_ptr<CardSelectionResult> selectionResult =
        cardSelectionManager->processCardSelectionScenario(reader);

    /* Check the selection result */
    if (selectionResult->getActiveSmartCard() == nullptr) {
        logger->error("The selection of the card failed\n");
        return -1;
    }

    /* Get the SmartCard resulting of the selection */
    std::shared_ptr<SmartCard> smartCard = selectionResult->getActiveSmartCard();

    logger->info("= SmartCard = %\n", smartCard);

    return 0;
}
