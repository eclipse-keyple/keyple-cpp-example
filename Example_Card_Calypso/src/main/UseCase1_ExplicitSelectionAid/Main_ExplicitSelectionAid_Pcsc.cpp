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

/* Keyple Plugin Pcsc */
#include "PcscPlugin.h"
#include "PcscPluginFactory.h"
#include "PcscPluginFactoryBuilder.h"
#include "PcscReader.h"
#include "PcscSupportedContactlessProtocol.h"

/* Keyple Cpp Example */
#include "CalypsoConstants.h"
#include "ConfigurationUtil.h"

using namespace keyple::card::calypso;
using namespace keyple::core::service;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp;
using namespace keyple::core::util::cpp::exception;
using namespace keyple::core::util::protocol;
using namespace keyple::plugin::pcsc;

/**
 *
 *
 * <h1>Use Case Calypso 1 – Explicit Selection Aid (PC/SC)</h1>
 *
 * <p>We demonstrate here the direct selection of a Calypso card inserted in a reader. No
 * observation of the reader is implemented in this example, so the card must be present in the
 * reader before the program is launched.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Checks if an ISO 14443-4 card is in the reader, enables the card selection manager.
 *   <li>Attempts to select the specified card (here a Calypso card characterized by its AID) with
 *       an AID-based application selection scenario, including reading a file record.
 *   <li>Output the collected data (FCI, ATR and file record content).
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in a runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_ExplicitSelectionAid_Pcsc {};
static std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_ExplicitSelectionAid_Pcsc));

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

    std::shared_ptr<Reader> cardReader =
        ConfigurationUtil::getCardReader(plugin, ConfigurationUtil::CARD_READER_NAME_REGEX);

    std::dynamic_pointer_cast<ConfigurableReader>(cardReader)
        ->activateProtocol(PcscSupportedContactlessProtocol::ISO_14443_4.getName(),
                           ContactlessCardCommonProtocol::ISO_14443_4.getName());

    /* Get the Calypso card extension service */
    std::shared_ptr<CalypsoExtensionService> cardExtension = CalypsoExtensionService::getInstance();

    /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(cardExtension);

    logger->info("=============== " \
                 "UseCase Calypso #1: AID based explicit selection " \
                 "==================");

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
    std::shared_ptr<CalypsoCardSelection> selection = cardExtension->createCardSelection();
    selection->filterByDfName(CalypsoConstants::AID)
              .acceptInvalidatedCard()
              .prepareReadRecord(CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER,
                                 CalypsoConstants::RECORD_NUMBER_1);
    cardSelectionManager->prepareSelection(selection);

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
    const auto calypsoCard =
        std::dynamic_pointer_cast<CalypsoCard>(selectionResult->getActiveSmartCard());

    logger->info("= SmartCard = %\n", calypsoCard);

    logger->info("Calypso Serial Number = %\n",
                     ByteArrayUtil::toHex(calypsoCard->getApplicationSerialNumber()));

    logger->info("File SFI %h, rec 1: FILE_CONTENT = %\n",
                 StringUtils::format("%02X", CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER),
                 calypsoCard->getFileBySfi(CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER));

    logger->info("= #### End of the Calypso card processing\n");

    return 0;
}