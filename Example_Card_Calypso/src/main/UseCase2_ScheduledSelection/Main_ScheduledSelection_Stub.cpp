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

/* Keyple Card Calypso */
#include "CalypsoExtensionService.h"

/* Keyple Core Service */
#include "ConfigurableReader.h"
#include "ObservableReader.h"
#include "SmartCardService.h"
#include "SmartCardServiceProvider.h"

/* Keyple Core Util */
#include "ByteArrayUtil.h"
#include "ContactCardCommonProtocol.h"
#include "ContactlessCardCommonProtocol.h"
#include "IllegalStateException.h"
#include "LoggerFactory.h"
#include "StringUtils.h"
#include "Thread.h"

/* Keyple Plugin Stub */
#include "StubPlugin.h"
#include "StubPluginFactoryBuilder.h"
#include "StubReader.h"

/* Keyple Cpp Example */
#include "CalypsoConstants.h"
#include "CardReaderObserver.h"
#include "StubSmartCardFactory.h"


using namespace keyple::card::calypso;
using namespace keyple::core::service;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp;
using namespace keyple::core::util::cpp::exception;
using namespace keyple::core::util::protocol;
using namespace keyple::plugin::stub;

/**
 *
 *
 * <h1>Use Case Generic 2 – Scheduled Selection (Stub)</h1>
 *
 * <p>We demonstrate here the selection of a Calypso card using a scheduled scenario. The selection
 * operations are prepared in advance with the card selection manager and the Calypso extension
 * service, then the reader is observed. When a card is inserted, the prepared selection scenario is
 * executed and the observer is notified of a card insertion event including the selection data
 * collected during the selection process.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Schedule a selection scenario over an observable reader to target a specific card (here a
 *       Calypso card characterized by its AID) and including the reading of a file record.
 *   <li>Start the observation and wait for a card insertion.
 *   <li>Simulate the card insertion.
 *   <li>Within the reader event handler:
 *       <ul>
 *         <li>Output collected card data (FCI and ATR).
 *         <li>Close the physical channel.
 *       </ul>
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 */
class Main_ScheduledSelection_Stub {};
static std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_ScheduledSelection_Stub));

static const std::string CARD_READER_NAME = "Stub card reader";

int main()
{
    /* Get the instance of the SmartCardService (singleton pattern) */
    std::shared_ptr<SmartCardService> smartCardService = SmartCardServiceProvider::getService();

    /*
     * Register the StubPlugin with the SmartCardService, plug a Calypso card stub
     * get the corresponding generic plugin in return.
     */
    std::shared_ptr<StubPluginFactory> pluginFactory =
        StubPluginFactoryBuilder::builder()
            ->withStubReader(CARD_READER_NAME, true, nullptr)
            .withMonitoringCycleDuration(100)
            .build();
    std::shared_ptr<Plugin> plugin = smartCardService->registerPlugin(pluginFactory);

    /* Get the Calypso card extension service */
    auto cardExtension = CalypsoExtensionService::getInstance();

    /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(cardExtension);

    std::shared_ptr<Reader> cardReader = plugin->getReader(CARD_READER_NAME);

    /* Activate the ISO14443 card protocol */
    std::dynamic_pointer_cast<ConfigurableReader>(cardReader)
        ->activateProtocol(ContactlessCardCommonProtocol::ISO_14443_4.getName(),
                           ContactlessCardCommonProtocol::ISO_14443_4.getName());

    logger->info("=============== " \
                 "UseCase Generic #2: scheduled selection " \
                 "==================\n");
    logger->info("= #### Select application with AID = '%'\n", CalypsoConstants::AID);

    /* Get the core card selection manager */
    std::shared_ptr<CardSelectionManager> cardSelectionManager =
        smartCardService->createCardSelectionManager();

    /*
     * Create a card selection using the Calypso card extension.
     * Select the card and read the record 1 of the file ENVIRONMENT_AND_HOLDER
     */
    std::shared_ptr<CalypsoCardSelection>  cardSelection = cardExtension->createCardSelection();
    cardSelection->acceptInvalidatedCard()
                  .filterByCardProtocol(ContactlessCardCommonProtocol::ISO_14443_4.getName())
                  .filterByDfName(CalypsoConstants::AID)
                  .prepareReadRecord(CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER,
                                     CalypsoConstants::RECORD_NUMBER_1);

    /*
     * Prepare the selection by adding the created Calypso selection to the card selection scenario
     */
    cardSelectionManager->prepareSelection(cardSelection);

    /*
     * Schedule the selection scenario, request notification only if the card matches the selection
     * case.
     */
    auto observableReader = std::dynamic_pointer_cast<ObservableReader>(cardReader);
    cardSelectionManager->scheduleCardSelectionScenario(
        observableReader,
        ObservableCardReader::DetectionMode::REPEATING,
        ObservableCardReader::NotificationMode::MATCHED_ONLY);

    /* Create and add an observer for this reader */
    auto cardReaderObserver = std::make_shared<CardReaderObserver>(cardReader,cardSelectionManager);
    observableReader->setReaderObservationExceptionHandler(cardReaderObserver);
    observableReader->addObserver(cardReaderObserver);
    observableReader->startCardDetection(ObservableCardReader::DetectionMode::REPEATING);

    logger->info("= #### Wait for a card. The default AID based selection to be processed as soon" \
                 " as the card is detected\n");

    /* Wait a while. */
    Thread::sleep(100);

    logger->info("Insert stub card\n");
    std::dynamic_pointer_cast<StubReader>(cardReader->getExtension(typeid(StubReader)))
        ->insertCard(StubSmartCardFactory::getStubCard());

    /* Wait a while. */
    Thread::sleep(1000);

    logger->info("Remove stub card\n");
    std::dynamic_pointer_cast<StubReader>(cardReader->getExtension(typeid(StubReader)))
        ->removeCard();

    /* Unregister plugin */
    smartCardService->unregisterPlugin(plugin->getName());

    logger->info("Exit program\n");

    return 0;
}
