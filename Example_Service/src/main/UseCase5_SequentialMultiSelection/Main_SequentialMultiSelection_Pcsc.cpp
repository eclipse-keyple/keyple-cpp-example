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
#include "ByteArrayUtil.h"
#include "IllegalStateException.h"
#include "LoggerFactory.h"

/* Keyple Core Service */
#include "ConfigurableReader.h"
#include "ObservableReader.h"
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
 * <h1>Use Case Generic 5 – Sequential selections based on an AID prefix (PC/SC)</h1>
 *
 * <p>We demonstrate here the selection of two applications in a single card, with both applications
 * selected sequentially using the same AID and the "FIRST" and "NEXT" navigation options.<br>
 * The result of the first selection is available to the application before the second selection is
 * executed.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Check if a ISO 14443-4 card is in the reader, select a card (here a card having two
 *       applications whose DF Names are prefixed by a specific AID [see AID_KEYPLE_PREFIX]).
 *   <li>Run an AID based application selection scenario (first occurrence).
 *   <li>Output collected smart card data (FCI and power-on data).
 *   <li>Run an AID based application selection scenario (next occurrence).
 *   <li>Output collected smart card data (FCI and power-on data).
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_SequentialMultiSelection_Pcsc {};
const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_SequentialMultiSelection_Pcsc));

/**
   * Performs the selection for the provided reader and logs its result.
   *
   * <p>The card selection manager must have been previously assigned a selection case.
   *
   * @param reader The reader.
   * @param cardSelectionsService The card selection manager.
   * @param index An int indicating the selection rank.
   */
static void doAndAnalyseSelection(std::shared_ptr<Reader> reader,
                                  std::shared_ptr<CardSelectionManager> cardSelectionsService,
                                  const int index)
{
    std::shared_ptr<CardSelectionResult> cardSelectionsResult =
        cardSelectionsService->processCardSelectionScenario(reader);
    if (cardSelectionsResult->getActiveSmartCard() != nullptr) {
        std::shared_ptr<SmartCard> smartCard = cardSelectionsResult->getActiveSmartCard();

        logger->info("The card matched the selection %\n", index);

        const std::string& powerOnData = smartCard->getPowerOnData();
        const std::string selectApplicationResponse =
            ByteArrayUtil::toHex(smartCard->getSelectApplicationResponse());

        logger->info("Selection status for case %: \n" \
                     "\t\tpower-on data: %\n" \
                     "\t\tSelect Application response: %\n",
                     index,
                     powerOnData,
                     selectApplicationResponse);
    } else {
        logger->info("The selection did not match for case %\n", index);
    }
}

int main()
{
    logger->setLoggerLevel(Logger::Level::logTrace);

    /* Get the instance of the SmartCardService (singleton pattern) */
    std::shared_ptr<SmartCardService> smartCardService = SmartCardServiceProvider::getService();

    /*
     * Register the PcscPlugin with the SmartCardService, get the corresponding generic plugin in
     * return.
     */
    std::shared_ptr<Plugin> plugin =
        smartCardService->registerPlugin(PcscPluginFactoryBuilder::builder()->build());

    /* Get the contactless reader whose name matches the provided regex */
    std::shared_ptr<Reader> reader =
        ConfigurationUtil::getCardReader(plugin, ConfigurationUtil::CONTACTLESS_READER_NAME_REGEX);

    /* Get the generic card extension service */
    std::shared_ptr<GenericExtensionService> cardExtension = GenericExtensionService::getInstance();

    /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(cardExtension);

    logger->info("=============== " \
                 "UseCase Generic #5: sequential selections based on an AID prefix " \
                 "===============");

    /* Check if a card is present in the reader */
    if (!reader->isCardPresent()) {
      throw IllegalStateException("No card is present in the reader.");
    }

    logger->info("= #### Select application with AID = '%'\n",
                 ConfigurationUtil::AID_KEYPLE_PREFIX);

    /* Get the core card selection manager */
    std::shared_ptr<CardSelectionManager> cardSelectionManager =
        smartCardService->createCardSelectionManager();

    /*
     * AID based selection: get the first application occurrence matching the AID, keep the
     * physical channel open
     */
    std::shared_ptr<GenericCardSelection> cardSelection = cardExtension->createCardSelection();
    cardSelection->filterByDfName(ConfigurationUtil::AID_KEYPLE_PREFIX);
    cardSelection->setFileOccurrence(GenericCardSelection::FileOccurrence::FIRST);

    /*
     * Prepare the selection by adding the created generic selection to the card selection scenario
     */
    cardSelectionManager->prepareSelection(cardSelection);

    /* Do the selection and display the result */
    doAndAnalyseSelection(reader, cardSelectionManager, 1);

    /*
     * New selection: get the next application occurrence matching the same AID, close the
     * physical channel after
     */
    cardSelection = cardExtension->createCardSelection();
    cardSelection->filterByDfName(ConfigurationUtil::AID_KEYPLE_PREFIX);
    cardSelection->setFileOccurrence(GenericCardSelection::FileOccurrence::NEXT);

    /*
     * Prepare the selection by adding the created generic selection to the card selection scenario.
     */
    cardSelectionManager->prepareSelection(cardSelection);

    /* Close the channel after the selection */
    cardSelectionManager->prepareReleaseChannel();

    /* Do the selection and display the result */
    doAndAnalyseSelection(reader, cardSelectionManager, 2);

    logger->info("= #### End of the generic card processing\n");

    return 0;
}
