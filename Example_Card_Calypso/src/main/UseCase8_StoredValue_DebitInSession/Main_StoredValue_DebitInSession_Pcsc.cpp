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
 * <h1>Use Case Calypso 4 – Calypso Card authentication (PC/SC)</h1>
 *
 * <p>We demonstrate here the debit of the Stored Value counter of a Calypso card.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Checks if an ISO 14443-4 card is in the reader, enables the card selection manager.
 *   <li>Attempts to select a Calypso SAM (C1) in the contact reader.
 *   <li>Attempts to select the specified card (here a Calypso card characterized by its AID) with
 *       an AID-based application selection scenario.
 *   <li>Creates a {@link CardTransactionManager} using {@link CardSecuritySetting} referencing the
 *       SAM profile defined in the card resource service.
 *   <li>Displays the Stored Value status, debits the Store Value within a Secure Session.
 * </ul>
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 */
class Main_StoredValue_DebitInSession_Pcsc {};
static const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_StoredValue_DebitInSession_Pcsc));

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

    logger->info("=============== UseCase Calypso #8: Stored Value debit ==================\n");

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
    cardSecuritySetting->enableSvLoadAndDebitLog();

    /* Performs file reads using the card transaction manager in non-secure mode. */
    std::shared_ptr<CardTransactionManager> cardTransaction =
        calypsoCardService->createCardTransaction(cardReader, calypsoCard, cardSecuritySetting);
    cardTransaction->prepareSvGet(SvOperation::DEBIT, SvAction::DO)
                    .processOpening(WriteAccessLevel::DEBIT);

    /* Display the current SV status */
    logger->info("Current SV status (SV Get for DEBIT):\n");
    logger->info(". Balance = %\n", calypsoCard->getSvBalance());
    logger->info(". Last Transaction Number = %\n", calypsoCard->getSvLastTNum());

    /* Display the load and debit records. */
    logger->info(". Load log record = %\n", calypsoCard->getSvLoadLogRecord());
    logger->info(". Debit log record = %\n", calypsoCard->getSvDebitLogLastRecord());

    /* Prepare an SV Debit of 2 units */
    cardTransaction->prepareSvDebit(2).prepareReleaseCardChannel().processClosing();

    logger->info("The Secure Session ended successfully, the stored value has been debited by 2 " \
                 "units\n");

    logger->info("= #### End of the Calypso card processing\n");

    return 0;
}
