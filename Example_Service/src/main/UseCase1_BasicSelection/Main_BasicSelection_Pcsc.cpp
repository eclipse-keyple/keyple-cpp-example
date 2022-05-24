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
 * <h1>Use Case Generic 1 – Basic Selection (PC/SC)</h1>
 *
 * <p>We demonstrate here a selection of cards without any condition related to the card itself. Any
 * card able to communicate with the reader must lead to a "selected" state.<br>
 * Note that in this case, no APDU "select application" is sent to the card.<br>
 * However, upon selection, an APDU command specific to Global Platform compliant cards is sent to
 * the card and may fail depending on the type of card presented.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Check if a ISO 14443-4 card is in the reader, select a card (a GlobalPlatform compliant
 *       card is expected here [e.g. EMV card or Javacard]).
 *   <li>Run a selection scenario without filter.
 *   <li>Output the collected smart card data (power-on data).
 *   <li>Send a additional APDUs to the card (get Card Production Life Cycle data [CPLC]).
 * </ul>
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_BasicSelection_Pcsc {};
const std::unique_ptr<Logger> logger = LoggerFactory::getLogger(typeid(Main_BasicSelection_Pcsc));

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
                 "UseCase Generic #1: basic card selection " \
                 "==================\n");

    /* Check if a card is present in the reader */
    if (!reader->isCardPresent()) {
        logger->error("No card is present in the reader\n");
        return 0;
    }

    logger->info("= #### Select the card with no conditions\n");

    /* Get the core card selection manager */
    std::shared_ptr<CardSelectionManager> cardSelectionManager =
        smartCardService->createCardSelectionManager();

    /*
     * Create a card selection using the generic card extension without specifying any filter
     * (protocol/power-on data/DFName).
     */
    std::shared_ptr<CardSelection> cardSelection = cardExtension->createCardSelection();

    /*
     * Prepare the selection by adding the created generic selection to the card selection scenario
     */
    cardSelectionManager->prepareSelection(cardSelection);

    /* Actual card communication: run the selection scenario */
    std::shared_ptr<CardSelectionResult> selectionResult =
        cardSelectionManager->processCardSelectionScenario(reader);

    /* Check the selection result */
    if (selectionResult->getActiveSmartCard() == nullptr) {
        throw IllegalStateException("The selection of the card failed.");
    }

    /* Get the SmartCard resulting of the selection */
    std::shared_ptr<SmartCard> smartCard = selectionResult->getActiveSmartCard();

    logger->info("= SmartCard = %\n", smartCard);

    /* Execute an APDU to get CPLC Data (cf. Global Platform Specification) */
    const std::vector<uint8_t> cplcApdu = ByteArrayUtil::fromHex("80CA9F7F00");

    const std::vector<std::string> apduResponses =
        cardExtension->createCardTransaction(reader, smartCard)
                     ->prepareApdu(cplcApdu)
                      .prepareReleaseChannel()
                      .processApdusToHexStrings();

    logger->info("CPLC Data: '%'\n", apduResponses[0]);

    logger->info("= #### End of the generic card processing\n");

    return 0;
}
