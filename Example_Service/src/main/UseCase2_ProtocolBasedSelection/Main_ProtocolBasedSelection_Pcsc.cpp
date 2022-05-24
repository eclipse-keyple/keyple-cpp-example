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

/* Keyple Card Generic */
#include "GenericCardSelectionAdapter.h"
#include "GenericExtensionService.h"

 /* Keyple Core Util */
#include "LoggerFactory.h"

/* Keyple Core Service */
#include "ConfigurableReader.h"
#include "SmartCardService.h"
#include "SmartCardServiceProvider.h"

/* Keyple Plugin Pcsc */
#include "PcscPluginFactoryBuilder.h"

/* Keyple Cpp Example */
#include "ConfigurationUtil.h"

using namespace keyple::card::generic;
using namespace keyple::core::service;
using namespace keyple::core::util::cpp;
using namespace keyple::plugin::pcsc;

/**
 *
 *
 * <h1>Use Case Generic 2 – Protocol Based Selection (PC/SC)</h1>
 *
 * <p>We demonstrate here a selection of cards with the only condition being the type of
 * communication protocol they use, in this case the Mifare Classic. Any card of the Mifare Classic
 * type must lead to a "selected" status, any card using another protocol must be ignored.<br>
 * Note that in this case, no APDU "select application" is sent to the card.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Check if a ISO 14443-4 card is in the reader, select a card (a Mifare Classic card is
 *       expected here).
 *   <li>Run a selection scenario with the MIFARE CLASSIC protocol filter.
 *   <li>Output the collected smart card data (power-on data).
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_ProtocolBasedSelection_Pcsc {};
const std::string MIFARE_CLASSIC = "MIFARE_CLASSIC";
const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_ProtocolBasedSelection_Pcsc));

int main()
{
    /* Get the instance of the SmartCardService (singleton pattern) */
    std::shared_ptr<SmartCardService> smartCardService = SmartCardServiceProvider::getService();

    /*
     * Register the PcscPlugin with the SmartCardService, get the corresponding generic plugin in
     * return.
     */
    std::shared_ptr<Plugin> plugin =
        smartCardService->registerPlugin(PcscPluginFactoryBuilder::builder()->build());

    /* Get the generic card extension service */
    std::shared_ptr<GenericExtensionService> cardExtension = GenericExtensionService::getInstance();

    /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(cardExtension);

    /* Get the contactless reader whose name matches the provided regex */
    std::shared_ptr<Reader> reader =
        ConfigurationUtil::getCardReader(plugin, ConfigurationUtil::CONTACTLESS_READER_NAME_REGEX);

    std::dynamic_pointer_cast<ConfigurableReader>(reader)
        ->activateProtocol(MIFARE_CLASSIC, MIFARE_CLASSIC);

    logger->info("=============== " \
                 "UseCase Generic #2: protocol based card selection " \
                 "==================\n");

    /* Check if a card is present in the reader */
    if (!reader->isCardPresent()) {
        logger->error("No card is present in the reader\n");
        return 0;
    }

    logger->info("= #### Select the card if the protocol is '%'\n", MIFARE_CLASSIC);

    /* Get the core card selection manager */
    std::shared_ptr<CardSelectionManager> cardSelectionManager =
        smartCardService->createCardSelectionManager();

    /*  Create a card selection using the generic card extension and specifying a Mifare filter. */
    std::shared_ptr<GenericCardSelection> cardSelection = cardExtension->createCardSelection();
    cardSelection->filterByCardProtocol(MIFARE_CLASSIC);

    /*
     * Prepare the selection by adding the created generic selection to the card selection scenario
     */
    cardSelectionManager->prepareSelection(cardSelection);

    /* Actual card communication: run the selection scenario */
    std::shared_ptr<CardSelectionResult> selectionResult =
        cardSelectionManager->processCardSelectionScenario(reader);

    /* Check the selection result */
    if (selectionResult->getActiveSmartCard() == nullptr) {
        logger->error("The selection of the card failed\n");
        return 0;
    }

    /* Get the SmartCard resulting of the selection */
    std::shared_ptr<SmartCard> smartCard = selectionResult->getActiveSmartCard();

    logger->info("= SmartCard = %\n", smartCard);

    logger->info("= #### End of the generic card processing\n");

    return 0;
}
