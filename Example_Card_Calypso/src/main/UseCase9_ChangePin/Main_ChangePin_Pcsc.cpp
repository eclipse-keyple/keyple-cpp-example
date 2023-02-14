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

/* Keyple Core Util */
#include "HexUtil.h"
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

/* Keyple Cpp Example */
#include "CalypsoConstants.h"
#include "ConfigurationUtil.h"

using namespace calypsonet::terminal::reader;
using namespace keyple::card::calypso;
using namespace keyple::core::service;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp;
using namespace keyple::core::util::cpp::exception;
using namespace keyple::core::util::protocol;
using namespace keyple::plugin::pcsc;

/**
 * <h1>Use Case Calypso 9 – Calypso Card Change PIN (PC/SC)</h1>
 *
 * <p>We demonstrate here the various operations around the PIN code checking.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Checks if an ISO 14443-4 card is in the reader, enables the card selection manager.
 *   <li>Attempts to select a Calypso SAM (C1) in the contact reader.
 *   <li>Attempts to select the specified card (here a Calypso card characterized by its AID) with
 *       an AID-based application selection scenario.
 *   <li>Creates a CardTransactionManager using CardSecuritySetting referencing the selected SAM.
 *   <li>Ask for the new PIN code.
 *   <li>Change the PIN code.
 *   <li>Verify the PIN code.
 *   <li>Close the card transaction.
 * </ul>
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 */
class Main_ChangePin_Pcsc {};
static const std::unique_ptr<Logger> logger = LoggerFactory::getLogger(typeid(Main_ChangePin_Pcsc));

int main()
{
    /* Get the instance of the SmartCardService */
    std::shared_ptr<SmartCardService> smartCardService = SmartCardServiceProvider::getService();

    /* Register the PcscPlugin, get the corresponding generic plugin in return */
    std::shared_ptr<Plugin> plugin =
        smartCardService->registerPlugin(PcscPluginFactoryBuilder::builder()->build());

    /* Get the Calypso card extension service */
    std::shared_ptr<CalypsoExtensionService> calypsoCardService = CalypsoExtensionService::getInstance();

    /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(calypsoCardService);

    /* Get the card and SAM readers whose name matches the provided regexs */
    std::shared_ptr<CardReader> cardReader =
        ConfigurationUtil::getCardReader(plugin, ConfigurationUtil::CARD_READER_NAME_REGEX);
    std::shared_ptr<CardReader> samReader =
        ConfigurationUtil::getSamReader(plugin, ConfigurationUtil::SAM_READER_NAME_REGEX);

    logger->info("=============== "
                 "UseCase Calypso #5: Calypso card Verify PIN "
                 "==================\n");

    /* Check if a card is present in the reader */
    if (!cardReader->isCardPresent()) {
        throw IllegalStateException("No card is present in the reader.");
    }

    /* Get the Calypso SAM SmartCard after selection. */
    std::shared_ptr<CalypsoSam> calypsoSam = ConfigurationUtil::getSam(samReader);

    logger->info("= SAM = %\n", calypsoSam);

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

    /* Create security settings that reference the SAM */
    std::shared_ptr<CardSecuritySetting> cardSecuritySetting =
        CalypsoExtensionService::getInstance()->createCardSecuritySetting();
    cardSecuritySetting->setControlSamResource(samReader, calypsoSam)
                        .setPinVerificationCipheringKey(
                            CalypsoConstants::PIN_VERIFICATION_CIPHERING_KEY_KIF,
                            CalypsoConstants::PIN_VERIFICATION_CIPHERING_KEY_KVC)
                        .setPinModificationCipheringKey(
                            CalypsoConstants::PIN_MODIFICATION_CIPHERING_KEY_KIF,
                            CalypsoConstants::PIN_MODIFICATION_CIPHERING_KEY_KVC);

    /* Create the card transaction manager in secure mode. */
    std::shared_ptr<CardTransactionManager> cardTransaction =
        calypsoCardService->createCardTransaction(cardReader, calypsoCard, cardSecuritySetting);

    /* Short delay to allow logs to be displayed before the prompt */
    Thread::sleep(2000);

    std::string inputString;
    std::vector<uint8_t> newPinCode(4);
    bool validPinCodeEntered = false;

    do {
        std::cout << "Enter new PIN code (4 numeric digits): ";
        std::cin >> inputString;

        if (!StringUtils::matches(inputString, "[0-9]{4}")) {
            std::cout << "Invalid PIN code." << std::endl;
        } else {
            newPinCode = std::vector<uint8_t>(inputString.c_str(), inputString.c_str() + 4);
            validPinCodeEntered = true;
        }

    } while (!validPinCodeEntered);

    /* Change the PIN (correct) */
    cardTransaction->processChangePin(newPinCode);

    logger->info("PIN code value successfully updated to %\n", inputString);

    /* Verification of the PIN */
    cardTransaction->processVerifyPin(newPinCode);
    logger->info("Remaining attempts: %\n", calypsoCard->getPinAttemptRemaining());

    logger->info("PIN % code successfully presented.\n", inputString);

    cardTransaction->prepareReleaseCardChannel();

    logger->info("= #### End of the Calypso card processing\n");

    return 0;
}
