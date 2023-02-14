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
#include "ConfigurableCardReader.h"

/* Keyple Card Calypso */
#include "CalypsoExtensionService.h"

/* Keyple Core Service */
#include "ConfigurableReader.h"
#include "SmartCardService.h"
#include "SmartCardServiceProvider.h"

/* Keyple Core Util */
#include "ContactCardCommonProtocol.h"
#include "HexUtil.h"
#include "IllegalStateException.h"
#include "LoggerFactory.h"
#include "StringUtils.h"

/* Keyple Plugin Pcsc */
#include "PcscPlugin.h"
#include "PcscPluginFactory.h"
#include "PcscPluginFactoryBuilder.h"
#include "PcscSupportedContactlessProtocol.h"

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
 * <h1>Use Case ‘Calypso 3 – Selection a Calypso card Revision 1 (BPRIME protocol) (PC/SC)</h1>
 *
 * <p>We demonstrate here the direct selection of a Calypso card Revision 1 (Innovatron / B Prime
 * protocol) inserted in a reader. No observation of the reader is implemented in this example, so
 * the card must be present in the reader before the program is launched.
 *
 * <p>No AID is used here, the reading of the card data is done without any prior card selection
 * command as defined in the ISO standard.
 *
 * <p>The card selection (in the Keyple sense, i.e. retained to continue processing) is based on the
 * protocol.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Check if a ISO B Prime (Innovatron protocol) card is in the reader.
 *   <li>Send 2 additional APDUs to the card (one following the selection step, one after the
 *       selection, within a card transaction [without security here]).
 * </ul>
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 */
class Main_Rev1Selection_Pcsc {};
static const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_Rev1Selection_Pcsc));

int main()
{
    /* Get the instance of the SmartCardService */
    std::shared_ptr<SmartCardService> smartCardService = SmartCardServiceProvider::getService();

    /* Register the PcscPlugin, get the corresponding generic plugin in return */
    std::shared_ptr<Plugin> plugin =
        smartCardService->registerPlugin(PcscPluginFactoryBuilder::builder()->build());

    /* Get the contactless reader whose name matches the provided regex */
    std::shared_ptr<CardReader> cardReader =
        ConfigurationUtil::getCardReader(plugin, ConfigurationUtil::CARD_READER_NAME_REGEX);

    /* Activate Innovatron protocol. */
    std::dynamic_pointer_cast<ConfigurableCardReader>(cardReader)
        ->activateProtocol(PcscSupportedContactlessProtocol::INNOVATRON_B_PRIME_CARD.getName(),
                           ConfigurationUtil::INNOVATRON_CARD_PROTOCOL);

    /* Get the Calypso card extension service */
    std::shared_ptr<CalypsoExtensionService> calypsoCardService =
        CalypsoExtensionService::getInstance();

     /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(calypsoCardService);

    logger->info("=============== " \
                 "UseCase Calypso #3: selection of a rev1 card " \
                 "==================\n");
    logger->info("= Card Reader  NAME = %\n", cardReader->getName());

    /* Check if a card is present in the reader */
    if (!cardReader->isCardPresent()) {
        throw IllegalStateException("No card is present in the reader.");
    }

    logger->info("= #### Select the card by its INNOVATRON protocol (no AID)\n");

    /* Get the core card selection manager */
    std::shared_ptr<CardSelectionManager> cardSelectionManager =
        smartCardService->createCardSelectionManager();

    /*
     * Create a card selection using the Calypso card extension.
     * Prepare the selection by adding the created Calypso card selection to the card selection
     * scenario. No AID is defined, only the card protocol will be used to define the selection
     * case.
     */
    std::shared_ptr<CalypsoCardSelection> selection = calypsoCardService->createCardSelection();
    selection->acceptInvalidatedCard()
              .filterByCardProtocol(ConfigurationUtil::INNOVATRON_CARD_PROTOCOL)
              .prepareReadRecord(CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER,
                                 CalypsoConstants::RECORD_NUMBER_1);

    /* Actual card communication: run the selection scenario */
    const std::shared_ptr<CardSelectionResult> selectionResult =
        cardSelectionManager->processCardSelectionScenario(cardReader);

    /* Check the selection result */
    if (selectionResult->getActiveSmartCard() == nullptr) {
        throw IllegalStateException("The selection of the B Prime card failed.");
    }

    /* Get the SmartCard resulting of the selection */
    const auto calypsoCard =
        std::dynamic_pointer_cast<CalypsoCard>(selectionResult->getActiveSmartCard());

    logger->info("= SmartCard = %\n", calypsoCard);

    const std::string csn = HexUtil::toHex(calypsoCard->getApplicationSerialNumber());
    logger->info("Calypso Serial Number = %\n", csn);

    /* Performs file reads using the card transaction manager in non-secure mode */
    std::shared_ptr<CardTransactionManager> extension =
        calypsoCardService->createCardTransactionWithoutSecurity(cardReader, calypsoCard);
    extension->prepareReadRecord(CalypsoConstants::SFI_EVENT_LOG, 1)
              .prepareReleaseCardChannel()
              .processCommands();

    const std::string sfiEnvHolder = HexUtil::toHex(CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER);
    logger->info("File %h, rec 1: FILE_CONTENT = %\n",
                 sfiEnvHolder,
                 calypsoCard->getFileBySfi(CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER));

    const std::string sfiEventLog = HexUtil::toHex(CalypsoConstants::SFI_EVENT_LOG);
    logger->info("File %h, rec 1: FILE_CONTENT = %\n",
                 sfiEventLog,
                 calypsoCard->getFileBySfi(CalypsoConstants::SFI_EVENT_LOG));

    logger->info("= #### End of the Calypso card processing\n");

    return 0;
}
