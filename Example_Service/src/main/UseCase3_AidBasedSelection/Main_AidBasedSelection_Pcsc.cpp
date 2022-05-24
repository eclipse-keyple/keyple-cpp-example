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
#include "GenericExtensionService.h"

/* Keyple Core Util */
#include "ByteArrayUtil.h"
#include "IllegalStateException.h"
#include "LoggerFactory.h"

/* Keyple Core Service */
#include "SmartCardService.h"
#include "SmartCardServiceProvider.h"

/* Keyple Plugin Pcsc */
#include "PcscPluginFactoryBuilder.h"

/* Keyple Cpp Example */
#include "ConfigurationUtil.h"

using namespace keyple::card::generic;
using namespace keyple::core::service;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp;
using namespace keyple::core::util::cpp::exception;
using namespace keyple::plugin::pcsc;

/**
 *
 *
 * <h1>Use Case Generic 3 – AID Based Selection (PC/SC)</h1>
 *
 * <p>We present here a selection of cards including the transmission of a "select application" APDU
 * targeting a specific DF Name. Any card with an application whose DF Name starts with the provided
 * AID should lead to a "selected" state, any card with another DF Name should be ignored.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Check if a ISO 14443-4 card is in the reader, select a card with the specified AID (here
 *       the EMV PPSE AID).
 *   <li>Run a selection scenario with the DF Name filter.
 *   <li>Output the collected smart card data (power-on data).
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_AidBasedSelection_Pcsc {};
const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_AidBasedSelection_Pcsc));

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

    logger->info("=============== " \
                 "UseCase Generic #3: AID based card selection " \
                 "==================\n");

    /* Check if a card is present in the reader */
    if (!reader->isCardPresent()) {
        logger->error("No card is present in the reader\n");
        return 0;
    }

    logger->info("= #### Select the card if its DF Name matches '%'\n",
                 ConfigurationUtil::AID_EMV_PPSE);

    /* Get the core card selection manager */
    std::shared_ptr<CardSelectionManager> cardSelectionManager =
        smartCardService->createCardSelectionManager();

    /*  Create a card selection using the generic card extension and specifying a DfName filter. */
    std::shared_ptr<GenericCardSelection> cardSelection = cardExtension->createCardSelection();
    cardSelection->filterByDfName(ConfigurationUtil::AID_EMV_PPSE);

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
