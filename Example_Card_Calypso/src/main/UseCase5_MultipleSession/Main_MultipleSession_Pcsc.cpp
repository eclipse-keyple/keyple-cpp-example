/**************************************************************************************************
 * Copyright (c) 2022 Calypso Networks Association https://calypsonet.org/                        *
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
#include "ConfigurableCardReader.h"

/* Keyple Card Calypso */
#include "CalypsoExtensionService.h"

/* Keyple Core Service */
#include "ConfigurableReader.h"
#include "SmartCardService.h"
#include "SmartCardServiceProvider.h"

/* Keyple Core Util */
#include "HexUtil.h"
#include "ContactCardCommonProtocol.h"
#include "ContactlessCardCommonProtocol.h"
#include "IllegalStateException.h"
#include "LoggerFactory.h"
#include "StringUtils.h"

/* Keyple Plugin Pcsc */
#include "PcscPlugin.h"
#include "PcscPluginFactory.h"
#include "PcscPluginFactoryBuilder.h"
#include "PcscReader.h"
#include "PcscSupportedContactlessProtocol.h"

/* Keyple Core Resource */
#include "CardResource.h"
#include "CardResourceServiceProvider.h"

/* Keyple Cpp Example */
#include "CalypsoConstants.h"
#include "ConfigurationUtil.h"

using namespace calypsonet::terminal::reader;
using namespace keyple::card::calypso;
using namespace keyple::core::service;
using namespace keyple::core::service::resource;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp;
using namespace keyple::core::util::cpp::exception;
using namespace keyple::core::util::protocol;
using namespace keyple::plugin::pcsc;

/**
 * <h1>Use Case Calypso 5 – Multiple sessions (PC/SC)</h1>
 *
 * <p>We demonstrate here a simple way to bypass the card modification buffer limitation by using
 * the multiple session mode.
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
 *   <li>Prepares and executes a number of modification commands that exceeds the number of commands
 *       allowed by the card's modification buffer size.
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_MultipleSession_Pcsc {};
static const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_MultipleSession_Pcsc));

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

    /* Get the Calypso card extension service */
    std::shared_ptr<CalypsoExtensionService> calypsoCardService =
        CalypsoExtensionService::getInstance();

     /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(calypsoCardService);

    /* Get the contactless reader whose name matches the provided regex */
    const std::string pcscContactlessReaderName =
        ConfigurationUtil::getCardReaderName(plugin, ConfigurationUtil::CARD_READER_NAME_REGEX);
    std::shared_ptr<CardReader> cardReader = plugin->getReader(pcscContactlessReaderName);

    /* Configure the reader with parameters suitable for contactless operations. */
    std::dynamic_pointer_cast<PcscReader>(
        plugin->getReaderExtension(typeid(PcscReader), pcscContactlessReaderName))
            ->setContactless(true)
             .setIsoProtocol(PcscReader::IsoProtocol::T1)
             .setSharingMode(PcscReader::SharingMode::SHARED);
    std::dynamic_pointer_cast<ConfigurableCardReader>(cardReader)
        ->activateProtocol(PcscSupportedContactlessProtocol::ISO_14443_4.getName(),
                           ConfigurationUtil::ISO_CARD_PROTOCOL);


    /*
     * Configure the card resource service to provide an adequate SAM for future secure operations.
     * We suppose here, we use an Identive contact PC/SC reader as card reader.
     */
     ConfigurationUtil::setupCardResourceService(plugin,
                                                 ConfigurationUtil::SAM_READER_NAME_REGEX,
                                                 CalypsoConstants::SAM_PROFILE_NAME);

    logger->info("=============== UseCase Calypso #5: multiple sessions ==================\n");

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
    std::shared_ptr<CalypsoCardSelection> cardSelection = calypsoCardService->createCardSelection();
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

    try {
        /* Performs file reads using the card transaction manager in secure mode */
        std::shared_ptr<CardTransactionManager> cardTransaction =
            calypsoCardService->createCardTransaction(cardReader, calypsoCard, cardSecuritySetting);
        cardTransaction->processOpening(WriteAccessLevel::DEBIT);

        /*
         * Compute the number of append records (29 bytes) commands that will overflow the card
         * modifications buffer. Each append records will consume 35 (29 + 6) bytes in the
         * buffer.
         *
         * We'll send one more command to demonstrate the MULTIPLE mode
         */
        /* note: not all Calypso card have this buffer size */
        const int modificationsBufferSize = 430;

        const int nbCommands = (modificationsBufferSize / 35) + 1;

        logger->info("==== Send % Append Record commands. Modifications buffer capacity = % bytes" \
                     " i.e. % 29-byte commands ====\n",
                     nbCommands,
                     modificationsBufferSize,
                     modificationsBufferSize / 35);

        for (int i = 0; i < nbCommands; i++) {
            cardTransaction->prepareAppendRecord(
                CalypsoConstants::SFI_EVENT_LOG,
                HexUtil::toByteArray(CalypsoConstants::EVENT_LOG_DATA_FILL));
        }

        cardTransaction->prepareReleaseCardChannel().processClosing();
    } catch (const Exception& e) {
        (void)e;
    }

    /* Finally */
    try {
        CardResourceServiceProvider::getService()->releaseCardResource(samResource);
    } catch (const RuntimeException& e) {
        logger->error("Error during the card resource release: %\n", e.getMessage(), e);
    }

    logger->info("The secure session has ended successfully, all data has been written to the " \
                 "card's memory\n");

    logger->info("= #### End of the Calypso card processing\n");

    return 0;
}
