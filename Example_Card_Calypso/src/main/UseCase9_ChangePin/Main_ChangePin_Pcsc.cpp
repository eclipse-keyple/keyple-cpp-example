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

using namespace keyple::card::calypso;
using namespace keyple::core::service;
using namespace keyple::core::service::resource;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp;
using namespace keyple::core::util::cpp::exception;
using namespace keyple::core::util::protocol;
using namespace keyple::plugin::pcsc;

/**
 *
 *
 * <h1>Use Case Calypso 9 – Calypso Card Change PIN (PC/SC)</h1>
 *
 * <p>We demonstrate here the various operations around the PIN code checking.
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
 *   <li>Ask for the new PIN code.
 *   <li>Change the PIN code.
 *   <li>Verify the PIN code.
 *   <li>Close the card transaction.
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_ChangePin_Pcsc {};
static const std::unique_ptr<Logger> logger = LoggerFactory::getLogger(typeid(Main_ChangePin_Pcsc));

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
    std::shared_ptr<CalypsoExtensionService> cardExtension = CalypsoExtensionService::getInstance();

     /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(cardExtension);

    /*
     * Get and setup the card reader
     * We suppose here, we use a ASK LoGO contactless PC/SC reader as card reader.
     */
    std::shared_ptr<Reader> cardReader =
        ConfigurationUtil::getCardReader(plugin, ConfigurationUtil::CARD_READER_NAME_REGEX);


    /*
     * Configure the card resource service to provide an adequate SAM for future secure operations.
     * We suppose here, we use a Identive contact PC/SC reader as card reader.
     */
     ConfigurationUtil::setupCardResourceService(plugin,
                                                 ConfigurationUtil::SAM_READER_NAME_REGEX,
                                                 CalypsoConstants::SAM_PROFILE_NAME);

    logger->info("=============== "
                 "UseCase Calypso #5: Calypso card Verify PIN "
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
    std::shared_ptr<CalypsoCardSelection>  cardSelection = cardExtension->createCardSelection();
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

    logger->info("Calypso Serial Number = %\n",
                 ByteArrayUtil::toHex(calypsoCard->getApplicationSerialNumber()));

    /* Create the card transaction manager */
    std::shared_ptr<CardTransactionManager> cardTransaction = nullptr;

    /*
     * Create security settings that reference the same SAM profile requested from the card resource
     * service, specifying the key ciphering key parameters.
     */
    std::shared_ptr<CardResource> samResource =
        CardResourceServiceProvider::getService()
            ->getCardResource(CalypsoConstants::SAM_PROFILE_NAME);

    std::shared_ptr<CardSecuritySetting> cardSecuritySetting =
        CalypsoExtensionService::getInstance()->createCardSecuritySetting();
    cardSecuritySetting->setSamResource(samResource->getReader(),
                                        std::dynamic_pointer_cast<CalypsoSam>(
                                            samResource->getSmartCard()))
                        .setPinVerificationCipheringKey(
                            CalypsoConstants::PIN_VERIFICATION_CIPHERING_KEY_KIF,
                            CalypsoConstants::PIN_VERIFICATION_CIPHERING_KEY_KVC)
                        .setPinModificationCipheringKey(
                            CalypsoConstants::PIN_MODIFICATION_CIPHERING_KEY_KIF,
                            CalypsoConstants::PIN_MODIFICATION_CIPHERING_KEY_KVC);

    try {
      /* Create a secured card transaction */
      cardTransaction =
          cardExtension->createCardTransaction(cardReader, calypsoCard, cardSecuritySetting);

      /* Short delay to allow logs to be displayed before the prompt */
      Thread::sleep(2000);

        std::string inputString;
        std::vector<uint8_t> newPinCode(4);
        bool validPinCodeEntered = false;
        do {
                std::cout << "Enter new PIN code (4 numeric digits):";

                std::cin >> inputString;

                if (!StringUtils::matches(inputString, "[0-9]{4}")) {
                    std::cout << "Invalid PIN code" << std::endl;
                } else {
                    newPinCode = std::vector<uint8_t>(inputString.begin(), inputString.end());
                    validPinCodeEntered = true;
                }
        } while (!validPinCodeEntered);

        /* Change the PIN (correct) */
        cardTransaction->processChangePin(newPinCode);

        logger->info("PIN code value successfully updated to %\n", inputString);


        /* Verification of the PIN */
        cardTransaction->processVerifyPin(newPinCode);
        logger->info("Remaining attempts: %\n", calypsoCard->getPinAttemptRemaining());

        logger->info("PIN % code successfully presented\n", inputString);

        cardTransaction->prepareReleaseCardChannel();
    } catch (const Exception& e) {
        (void)e;
    }

    /* Finally */
    try {
        CardResourceServiceProvider::getService()->releaseCardResource(samResource);
    } catch (const RuntimeException& e) {
        logger->error("Error during the card resource release: %\n", e.getMessage(), e);
    }

    logger->info("= #### End of the Calypso card processing\n");

    return 0;
}
