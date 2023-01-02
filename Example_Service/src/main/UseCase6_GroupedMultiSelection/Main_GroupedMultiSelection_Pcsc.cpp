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

/* Keyple Core Util */
#include "HexUtil.h"
#include "IllegalStateException.h"
#include "LoggerFactory.h"

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
#include "PcscReader.h"
#include "PcscSupportedContactlessProtocol.h"

/* Keyple Cpp Example */
#include "ConfigurationUtil.h"

/* Calypsonet Terminal Reader */
#include "CardSelectionManager.h"

using namespace calypsonet::terminal::reader;
using namespace calypsonet::terminal::reader::selection;
using namespace keyple::card::generic;
using namespace keyple::core::service;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp;
using namespace keyple::plugin::pcsc;

/**
 * <h1>Use Case Generic 6 – Grouped selections based on an AID prefix (PC/SC)</h1>
 *
 * <p>We demonstrate here the selection of two applications in a single card, with both applications
 * selected using the same AID and the "FIRST" and "NEXT" navigation options but grouped in the same
 * selection process.<br>
 * Both selection results are available in the {@link CardSelectionResult} object returned by the
 * execution of the selection scenario.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Check if a ISO 14443-4 card is in the reader, select a card (here a card having two
 *       applications whose DF Names are prefixed by a specific AID [see AID_KEYPLE_PREFIX]).
 *   <li>Run a double AID based application selection scenario (first and next occurrence).
 *   <li>Output collected of all smart cards data (FCI and power-on data).
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_GroupedMultiSelection_Pcsc {};
const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_GroupedMultiSelection_Pcsc));

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

    /* Get the generic card extension service */
    std::shared_ptr<GenericExtensionService> genericCardService =
        GenericExtensionService::getInstance();

    /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(genericCardService);

    /* Get the contactless reader whose name matches the provided regex */
    const std::string pcscContactlessReaderName =
        ConfigurationUtil::getCardReaderName(plugin,
                                             ConfigurationUtil::CONTACTLESS_READER_NAME_REGEX);
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

    logger->info("=============== " \
                 "UseCase Generic #6: Grouped selections based on an AID prefix " \
                 "===============\n");

    /* Check if a card is present in the reader */
    if (!cardReader->isCardPresent()) {
      throw IllegalStateException("No card is present in the reader.");
    }

    logger->info("= #### Select application with AID = '%'\n",
                 ConfigurationUtil::AID_KEYPLE_PREFIX);

    /* Get the core card selection manager */
    std::shared_ptr<CardSelectionManager> cardSelectionManager =
        smartCardService->createCardSelectionManager();

    /* Set the multiple selection mode */
    cardSelectionManager->setMultipleSelectionMode();

    /*
     * First selection: get the first application occurrence matching the AID, keep the
     * physical channel open.
     * Prepare the selection by adding the created generic selection to the card selection scenario.
     */
    std::shared_ptr<GenericCardSelection> cardSelection = genericCardService->createCardSelection();
    cardSelection->filterByDfName(ConfigurationUtil::AID_KEYPLE_PREFIX);
    cardSelection->setFileOccurrence(GenericCardSelection::FileOccurrence::FIRST);
    cardSelectionManager->prepareSelection(cardSelection);

    /*
     * Second selection: get the next application occurrence matching the same AID, close the
     * physical channel after.
     * Prepare the selection by adding the created generic selection to the card selection scenario.
     */
    cardSelection = genericCardService->createCardSelection();
    cardSelection->filterByDfName(ConfigurationUtil::AID_KEYPLE_PREFIX);
    cardSelection->setFileOccurrence(GenericCardSelection::FileOccurrence::NEXT);
    cardSelectionManager->prepareSelection(cardSelection);

    /* Close the channel after the selection */
    cardSelectionManager->prepareReleaseChannel();

    std::shared_ptr<CardSelectionResult> cardSelectionsResult =
        cardSelectionManager->processCardSelectionScenario(cardReader);

    /* Log the result */
    for (const auto& entry : cardSelectionsResult->getSmartCards()) {
        std::shared_ptr<SmartCard> smartCard = entry.second;
        const std::string& powerOnData = smartCard->getPowerOnData();
        const std::string selectApplicationResponse =
            HexUtil::toHex(smartCard->getSelectApplicationResponse());
        const std::string selectionIsActive =
            smartCard == cardSelectionsResult->getActiveSmartCard() ? "true" : "false";

        logger->info("Selection status for selection (indexed %): \n" \
                     "\t\tActive smart card: %\n" \
                     "\t\tpower-on data: %\n" \
                     "\t\tSelect Application response: %\n",
                     entry.first,
                     selectionIsActive,
                     powerOnData,
                     selectApplicationResponse);
    }

    logger->info("= #### End of the generic card processing\n");

    return 0;
}
