/**************************************************************************************************
 * Copyright (c) 2023 Calypso Networks Association https://calypsonet.org/                        *
 *                                                                                                *
 * See the NOTICE file(s) distributed with this work for additional information regarding         *
 * copyright ownership.                                                                           *
 *                                                                                                *
 * This program and the accompanying materials are made available under the terms of the Eclipse  *
 * Public License 2.0 which is available at http://www.eclipse.org/legal/epl-2.0                  *
 *                                                                                                *
 * SPDX-License-Identifier: EPL-2.0                                                               *
 **************************************************************************************************/

/* Calypsonet Terminal Reader */
#include "CardReader.h"

/* Keyple Card Calypso */
#include "CalypsoExtensionService.h"

/* Keyple Core Service */
#include "ConfigurableReader.h"
#include "ObservableReader.h"
#include "SmartCardService.h"
#include "SmartCardServiceProvider.h"

/* Keyple Core Service Resource */
#include "CardResource.h"
#include "CardResourceServiceProvider.h"

/* Keyple Core Util */
#include "ContactCardCommonProtocol.h"
#include "ContactlessCardCommonProtocol.h"
#include "HexUtil.h"
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
#include "ConfigurationUtil.h"
#include "StubSmartCardFactory.h"

using namespace keyple::card::calypso;
using namespace keyple::core::service;
using namespace keyple::core::service::resource;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp;
using namespace keyple::core::util::cpp::exception;
using namespace keyple::core::util::protocol;
using namespace keyple::plugin::stub;

/**
 * <h1>Use Case Calypso 4 – Calypso Card authentication (Stub)</h1>
 *
 * <p>We demonstrate here the authentication of a Calypso card using a Secure Session in which a
 * file from the card is read. The read is certified by verifying the signature of the card by a
 * Calypso SAM.
 *
 * <p>Two readers are required for this example: a contactless reader for the Calypso Card, a
 * contact reader for the Calypso SAM.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Sets up the card resource service to provide a Calypso SAM (C1).
 *   <li>Checks if an ISO 14443-4 card is in the reader, enables the card selection manager.
 *   <li>Attempts to select the specified card (here a Calypso card characterized by its AID) with
 *       an AID-based application selection scenario.
 *   <li>Creates a {@link CardTransactionManager} using {@link CardSecuritySetting} referencing the
 *       SAM profile defined in the card resource service.
 *   <li>Read a file record in Secure Session.
 * </ul>
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 */
class Main_CardAuthentication_Stub {};
static const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_CardAuthentication_Stub));

static const std::string CARD_READER_NAME = "Stub card reader";
static const std::string SAM_READER_NAME = "Stub SAM reader";

int main()
{
    /* Get the instance of the SmartCardService */
    std::shared_ptr<SmartCardService> smartCardService = SmartCardServiceProvider::getService();

    /*
     * Register the StubPlugin with the SmartCardService, plug a Calypso card stub
     * get the corresponding generic plugin in return.
     */
    std::shared_ptr<StubPluginFactory> pluginFactory =
        StubPluginFactoryBuilder::builder()
            ->withStubReader(CARD_READER_NAME, true, StubSmartCardFactory::getStubCard())
            .withStubReader(SAM_READER_NAME, false, StubSmartCardFactory::getStubSam())
            .build();
    std::shared_ptr<Plugin> plugin = smartCardService->registerPlugin(pluginFactory);

    /* Get the Calypso card extension service */
    auto calypsoCardService = CalypsoExtensionService::getInstance();

    /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(calypsoCardService);

    /* Get and set up the card reader */
    std::shared_ptr<CardReader> cardReader = plugin->getReader(CARD_READER_NAME);

    /*
     * Configure the card resource service to provide an adequate SAM for future secure operations
     */
    ConfigurationUtil::setupCardResourceService(plugin,
                                                SAM_READER_NAME,
                                                CalypsoConstants::SAM_PROFILE_NAME);

    logger->info("=============== " \
                 "UseCase Calypso #4: Calypso card authentication " \
                 "==================\n");

    /* Check if a card is present in the reader */
    if (!cardReader->isCardPresent()) {
        throw IllegalStateException("No card is present in the reader.");
    }

    logger->info("= #### Select application with AID = '%'\n", CalypsoConstants::AID);

    /* Get the core card selection manager */
    std::shared_ptr<CardSelectionManager> cardSelectionManager =
        smartCardService->createCardSelectionManager();

    /*
     * Create a card selection using the Calypso card extension.
     * Prepare the selection by adding the created Calypso card selection to the card selection
     * scenario.
     */
    std::shared_ptr<CalypsoCardSelection>  cardSelection = calypsoCardService->createCardSelection();
    cardSelection->acceptInvalidatedCard()
                  .filterByDfName(CalypsoConstants::AID);
    cardSelectionManager->prepareSelection(cardSelection);

    /* Actual card communication: run the selection scenario */
    const std::shared_ptr<CardSelectionResult> selectionResult =
        cardSelectionManager->processCardSelectionScenario(cardReader);

    /* Check the selection result */
    if (selectionResult->getActiveSmartCard() == nullptr) {
        throw IllegalStateException("The selection of the application '" +
                                    CalypsoConstants::AID +
                                    "' failed.");
    }

    /* Get the SmartCard resulting of the selection */
    const std::shared_ptr<SmartCard> card = selectionResult->getActiveSmartCard();
    auto calypsoCard = std::dynamic_pointer_cast<CalypsoCard>(card);

    logger->info("= SmartCard = %\n", calypsoCard);

    const std::string csn = HexUtil::toHex(calypsoCard->getApplicationSerialNumber());
    logger->info("Calypso Serial Number = %\n", csn);

    /*
     * Create security settings that reference the same SAM profile requested from the card resource
     * service.
     */
    std::shared_ptr<CardResource> samResource =
        CardResourceServiceProvider::getService()
            ->getCardResource(CalypsoConstants::SAM_PROFILE_NAME);

    std::shared_ptr<CardSecuritySetting> cardSecuritySetting =
        CalypsoExtensionService::getInstance()->createCardSecuritySetting();
    cardSecuritySetting->setControlSamResource(samResource->getReader(),
                                               std::dynamic_pointer_cast<CalypsoSam>(
                                                   samResource->getSmartCard()));

    //try {
        /* Performs file reads using the card transaction manager in secure mode */
        std::shared_ptr<CardTransactionManager> cardTransaction =
            calypsoCardService->createCardTransaction(cardReader, calypsoCard, cardSecuritySetting);
        cardTransaction->prepareReadRecords(CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER,
                                            CalypsoConstants::RECORD_NUMBER_1,
                                            CalypsoConstants::RECORD_NUMBER_1,
                                            CalypsoConstants::RECORD_SIZE)
                        .processOpening(WriteAccessLevel::DEBIT)
                        .prepareReleaseCardChannel()
                        .processClosing();
    //} finally {
    try {
        CardResourceServiceProvider::getService()->releaseCardResource(samResource);
    } catch (const RuntimeException& e) {
        logger->error("Error during the card resource release: %\n", e.getMessage(), e);
    }
    //}

    logger->info("The Secure Session ended successfully, the card is authenticated and the data " \
                 "read are certified\n");

    const std::string sfiEnvHolder = HexUtil::toHex(CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER);
    logger->info("File %h, rec 1: FILE_CONTENT = %\n",
                 sfiEnvHolder,
                 calypsoCard->getFileBySfi(CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER));

    logger->info("= #### End of the Calypso card processing\n");

    return 0;
}
