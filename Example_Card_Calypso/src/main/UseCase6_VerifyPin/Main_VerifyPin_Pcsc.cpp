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
#include "SmartCardService.h"
#include "SmartCardServiceProvider.h"

/* Keyple Core Util */
#include "ContactCardCommonProtocol.h"
#include "ContactlessCardCommonProtocol.h"
#include "HexUtil.h"
#include "IllegalStateException.h"
#include "LoggerFactory.h"
#include "StringUtils.h"

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
 * <h1>Use Case Calypso 6 – Calypso Card Verify PIN (PC/SC)</h1>
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
 *   <li>Creates a CardTransactionManager without security.
 *   <li>Verify the PIN code in plain mode with the correct code, display the remaining attempts
 *       counter.
 *   <li>Creates a CardTransactionManager using CardSecuritySetting referencing the
 *       selected SAM.
 *   <li>Verify the PIN code in session in encrypted mode with the code, display the remaining
 *       attempts counter.
 *   <li>Verify the PIN code in session in encrypted mode with a bad code, display the remaining
 *       attempts counter.
 *   <li>Cancel the card transaction, re-open a new one.
 *   <li>Verify the PIN code in session in encrypted mode with the code, display the remaining
 *       attempts counter.
 *   <li>Close the card transaction.
 * </ul>
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 */
class Main_VerifyPin_Pcsc {};
static const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_VerifyPin_Pcsc));

int main()
{
    /* Get the instance of the SmartCardService */
    std::shared_ptr<SmartCardService> smartCardService = SmartCardServiceProvider::getService();

    /* Register the PcscPlugin, get the corresponding generic plugin in return */
    std::shared_ptr<Plugin> plugin =
        smartCardService->registerPlugin(PcscPluginFactoryBuilder::builder()->build());

    /* Get the Calypso card extension service */
    std::shared_ptr<CalypsoExtensionService> calypsoCardService =
        CalypsoExtensionService::getInstance();

     /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(calypsoCardService);

    /* Get the card and SAM readers whose name matches the provided regexs */
    std::shared_ptr<CardReader> cardReader =
        ConfigurationUtil::getCardReader(plugin, ConfigurationUtil::CARD_READER_NAME_REGEX);
    std::shared_ptr<CardReader> samReader =
        ConfigurationUtil::getSamReader(plugin, ConfigurationUtil::SAM_READER_NAME_REGEX);

    logger->info("=============== " \
                 "UseCase Calypso #5: Calypso card Verify PIN " \
                 "================== \n");

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

    /* Create security settings that reference the SAM */
    std::shared_ptr<CardSecuritySetting> cardSecuritySetting =
        CalypsoExtensionService::getInstance()->createCardSecuritySetting();
    cardSecuritySetting->setControlSamResource(samReader, calypsoSam);
    cardSecuritySetting->setPinVerificationCipheringKey(
                            CalypsoConstants::PIN_VERIFICATION_CIPHERING_KEY_KIF,
                            CalypsoConstants::PIN_VERIFICATION_CIPHERING_KEY_KVC);

    /* Create the card transaction manager in secure mode. */
    std::shared_ptr<CardTransactionManager> cardTransaction =
            calypsoCardService->createCardTransactionWithoutSecurity(cardReader, calypsoCard);

    /* Verification of the PIN (correct) out of a secure session in plain mode */
    cardTransaction->processVerifyPin(CalypsoConstants::PIN_OK);
    logger->info("Remaining attempts #1: %\n", calypsoCard->getPinAttemptRemaining());

    /* Create a secured card transaction */
    cardTransaction = calypsoCardService->createCardTransaction(cardReader,
                                                                calypsoCard,
                                                                cardSecuritySetting);

    /* Verification of the PIN (correct) out of a secure session in encrypted mode */
    cardTransaction->processVerifyPin(CalypsoConstants::PIN_OK);

    /* Log the current counter value (should be 3) */
    logger->info("Remaining attempts #2: %\n", calypsoCard->getPinAttemptRemaining());

    /* Verification of the PIN (incorrect) inside a secure session */
    cardTransaction->processOpening(WriteAccessLevel::DEBIT);

    try {
        /* Create a secured card transaction */
        cardTransaction->processVerifyPin(CalypsoConstants::PIN_KO);
    } catch (const Exception& ex) {
      logger->error("PIN Exception: %\n", ex.getMessage());
    }

    /* Log the current counter value (should be 2) */
    logger->error("Remaining attempts #3: %\n", calypsoCard->getPinAttemptRemaining());

    /* Verification of the PIN (correct) inside a secure session with reading of the counter */
    cardTransaction->prepareCheckPinStatus();
    cardTransaction->processOpening(WriteAccessLevel::DEBIT);

    /* Log the current counter value (should be 2) */
    logger->info("Remaining attempts #4: %\n", calypsoCard->getPinAttemptRemaining());

    cardTransaction->processVerifyPin(CalypsoConstants::PIN_OK);
    cardTransaction->prepareReleaseCardChannel();
    cardTransaction->processClosing();

    /* Log the current counter value (should be 3) */
    logger->info("Remaining attempts #5: %\n", calypsoCard->getPinAttemptRemaining());

    logger->info("The Secure Session ended successfully, the PIN has been verified\n");

    logger->info("= #### End of the Calypso card processing\n");

    return 0;
}
