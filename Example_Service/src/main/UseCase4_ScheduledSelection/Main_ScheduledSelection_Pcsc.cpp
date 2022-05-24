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

/* Keyple Core Service */
#include "ConfigurableReader.h"
#include "ObservableReader.h"
#include "SmartCardService.h"
#include "SmartCardServiceProvider.h"

/* Keyple Plugin Pcsc */
#include "PcscPluginFactoryBuilder.h"

/* Keyple Cpp Example */
#include "ConfigurationUtil.h"
#include "CardReaderObserver.h"

using namespace keyple::card::generic;
using namespace keyple::core::service;
using namespace keyple::core::util::cpp;
using namespace keyple::plugin::pcsc;

/**
 *
 *
 * <h1>Use Case Generic 4 – Scheduled Selection (PC/SC)</h1>
 *
 * <p>We present here a selection of ISO-14443-4 cards including the transmission of a "select
 * application" APDU targeting EMV banking cards (AID PPSE). Any contactless EMV card should lead to
 * a "selected" state, any card with another DF Name should be ignored.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Schedule a selection scenario over an observable reader to target a specific card (here a
 *       EMV contactless card).
 *   <li>Start the observation and wait for a card.
 *   <li>Within the reader event handler:
 *       <ul>
 *         <li>Output collected smart card data (FCI and power-on data).
 *         <li>Close the physical channel.
 *       </ul>
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 */
class Main_ScheduledSelection_Pcsc {};
const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_ScheduledSelection_Pcsc));

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

    /* Get the contactless reader whose name matches the provided regex */
    std::shared_ptr<Reader> reader =
        ConfigurationUtil::getCardReader(plugin, ConfigurationUtil::CONTACTLESS_READER_NAME_REGEX);

    /* Activate the ISO14443 card protocol */
    std::dynamic_pointer_cast<ConfigurableReader>(reader)
        ->activateProtocol("ISO_14443_4", "ISO_14443_4.name");

    /* Get the generic card extension service */
    std::shared_ptr<GenericExtensionService> cardExtension = GenericExtensionService::getInstance();

    /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(cardExtension);

    logger->info("=============== " \
                 "UseCase Generic #4: scheduled AID based selection " \
                 "===============\n");

    logger->info("= #### Select application with AID = '%'\n", ConfigurationUtil::AID_EMV_PPSE);

    /* Get the core card selection manager */
    std::shared_ptr<CardSelectionManager> cardSelectionManager =
        smartCardService->createCardSelectionManager();
    /* Create a card selection using the generic card extension */
    std::shared_ptr<GenericCardSelection> cardSelection = cardExtension->createCardSelection();
    cardSelection->filterByCardProtocol("ISO_14443_4");
    cardSelection->filterByDfName(ConfigurationUtil::AID_EMV_PPSE);

    /*
     * Prepare the selection by adding the created generic selection to the card selection scenario
     */
    cardSelectionManager->prepareSelection(cardSelection);

    /* Schedule the selection scenario */
    cardSelectionManager->scheduleCardSelectionScenario(
        std::dynamic_pointer_cast<ObservableReader>(reader),
        ObservableCardReader::DetectionMode::REPEATING,
        ObservableCardReader::NotificationMode::MATCHED_ONLY);

    /* Create and add an observer */
    auto cardReaderObserver = std::make_shared<CardReaderObserver>(reader, cardSelectionManager);
    auto observableReader = std::dynamic_pointer_cast<ObservableReader>(reader);
    observableReader->setReaderObservationExceptionHandler(cardReaderObserver);
    observableReader->addObserver(cardReaderObserver);
    observableReader->startCardDetection(ObservableCardReader::DetectionMode::REPEATING);

    logger->info("= #### Wait for a card. The AID based selection scenario will be processed as " \
                 "soon as a card is detected\n");

    /* Unregister plugin */
    smartCardService->unregisterPlugin(plugin->getName());

    logger->info("Exit program\n");

    return 0;
}
