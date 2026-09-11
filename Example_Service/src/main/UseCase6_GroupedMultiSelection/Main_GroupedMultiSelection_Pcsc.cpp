/* ****************************************************************************
 * Copyright (c) 2025 Calypso Networks Association https://calypsonet.org/    *
 *                                                                            *
 * See the NOTICE file(s) distributed with this work for additional           *
 * information regarding copyright ownership.                                 *
 *                                                                            *
 * This program and the accompanying materials are made available under the   *
 * terms of the Eclipse Distribution License 1.0 which is available at        *
 * https://www.eclipse.org/org/documents/edl-v10.php                          *
 *                                                                            *
 * SPDX-License-Identifier: BSD-3-Clause                                      *
 ******************************************************************************/

#include <exception>
#include <memory>

#include "common/ConfigurationUtil.hpp"

#include "keyple/card/generic/GenericCardSelectionExtension.hpp"
#include "keyple/card/generic/GenericExtensionService.hpp"
#include "keyple/core/service/Plugin.hpp"
#include "keyple/core/service/SmartCardService.hpp"
#include "keyple/core/service/SmartCardServiceProvider.hpp"
#include "keyple/core/util/HexUtil.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keyple/core/util/cpp/exception/IllegalStateException.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keyple/plugin/pcsc/PcscSupportedContactlessProtocol.hpp"
#include "keypop/reader/CardReader.hpp"
#include "keypop/reader/ConfigurableCardReader.hpp"
#include "keypop/reader/ReaderApiFactory.hpp"
#include "keypop/reader/selection/CardSelectionManager.hpp"
#include "keypop/reader/selection/CardSelectionResult.hpp"
#include "keypop/reader/selection/FileOccurrence.hpp"
#include "keypop/reader/selection/IsoCardSelector.hpp"
#include "keypop/reader/selection/spi/IsoSmartCard.hpp"

using keyple::card::generic::GenericCardSelectionExtension;
using keyple::card::generic::GenericExtensionService;
using keyple::core::service::Plugin;
using keyple::core::service::SmartCardService;
using keyple::core::service::SmartCardServiceProvider;
using keyple::core::util::HexUtil;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keyple::core::util::cpp::exception::IllegalStateException;
using keyple::plugin::pcsc::PcscPluginFactoryBuilder;
using keyple::plugin::pcsc::PcscReader;
using keyple::plugin::pcsc::PcscSupportedContactlessProtocol;
using keypop::reader::CardReader;
using keypop::reader::ConfigurableCardReader;
using keypop::reader::ReaderApiFactory;
using keypop::reader::selection::CardSelectionManager;
using keypop::reader::selection::CardSelectionResult;
using keypop::reader::selection::FileOccurrence;
using keypop::reader::selection::IsoCardSelector;
using keypop::reader::selection::spi::IsoSmartCard;

/**
 * <h1>Use Case Generic 6 – Grouped selections based on an AID prefix
 * (PC/SC)</h1>
 *
 * <p>We demonstrate here the selection of two applications in a single card,
 * with both applications selected using the same AID and the "FIRST" and "NEXT"
 * navigation options but grouped in the same selection process.<br>
 * Both selection results are available in the CardSelectionResult object
 * returned by the execution of the selection scenario.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Check if a ISO 14443-4 card is in the reader, select a card (here a
 *       card having two applications whose DF Names are prefixed by a specific
 *       AID [see AID_KEYPLE_PREFIX]).
 *   <li>Run a double AID based application selection scenario (first and next
 *       occurrence).
 *   <li>Output collected of all smart cards data (FCI and power-on data).
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_GroupedMultiSelection_Pcsc { };
const std::unique_ptr<Logger> logger
    = LoggerFactory::getLogger(typeid(Main_GroupedMultiSelection_Pcsc));

static int
runExample() {
    /* Get the instance of the SmartCardService (singleton pattern) */
    std::shared_ptr<SmartCardService> smartCardService(
        SmartCardServiceProvider::getService());

    /*
     * Register the PcscPlugin with the SmartCardService, get the corresponding
     * generic plugin in return.
     */
    std::shared_ptr<Plugin> plugin(smartCardService->registerPlugin(
        PcscPluginFactoryBuilder::builder()->build()));

    /* Get the generic card extension service */
    std::shared_ptr<GenericExtensionService> genericCardService
        = GenericExtensionService::getInstance();

    /*
     * Verify that the extension's API level is consistent with the current
     * service.
     */
    smartCardService->checkCardExtension(genericCardService);

    /* Get the contactless reader whose name matches the provided regex */
    std::shared_ptr<CardReader> cardReader(
        plugin->findReader(ConfigurationUtil::CONTACTLESS_READER_NAME_REGEX));

    /*
     * Configure the reader with parameters suitable for contactless operations.
     */
    std::dynamic_pointer_cast<PcscReader>(
        plugin->getReaderExtension(typeid(PcscReader), cardReader->getName()))
        ->setContactless(true)
        .setIsoProtocol(PcscReader::IsoProtocol::T1)
        .setSharingMode(PcscReader::SharingMode::SHARED);
    std::dynamic_pointer_cast<ConfigurableCardReader>(cardReader)
        ->activateProtocol(
            PcscSupportedContactlessProtocol::ISO_14443_4.getName(),
            ConfigurationUtil::ISO_CARD_PROTOCOL);

    logger->info("=============== "
                 "UseCase Generic #6: Grouped selections based on an AID "
                 "prefix "
                 "===============\n");

    /* Check if a card is present in the reader */
    if (!cardReader->isCardPresent()) {
        throw IllegalStateException("No card is present in the reader.");
    }

    logger->info(
        "= #### Select application with AID = '%'\n",
        ConfigurationUtil::AID_KEYPLE_PREFIX);

    std::shared_ptr<ReaderApiFactory> readerApiFactory(
        smartCardService->getReaderApiFactory());
    std::shared_ptr<CardSelectionManager> cardSelectionManager(
        readerApiFactory->createCardSelectionManager());

    std::shared_ptr<GenericCardSelectionExtension> genericCardSelectionExtension
        = GenericExtensionService::getInstance()
              ->createGenericCardSelectionExtension();

    /*
     * AID based selection: get the first application occurrence matching the
     * AID, keep the physical channel open.
     * Prepare the selection by adding the created generic selection to the card
     * selection scenario.
     */
    std::shared_ptr<IsoCardSelector> cardSelector1(
        readerApiFactory->createIsoCardSelector());
    cardSelector1->filterByDfName(ConfigurationUtil::AID_KEYPLE_PREFIX);
    cardSelector1->setFileOccurrence(FileOccurrence::FIRST);

    /*
     * Prepare the selection by adding the created generic selection to the card
     * selection scenario.
     */
    cardSelectionManager->prepareSelection(
        cardSelector1, genericCardSelectionExtension);

    std::shared_ptr<IsoCardSelector> cardSelector2(
        readerApiFactory->createIsoCardSelector());
    cardSelector1->filterByDfName(ConfigurationUtil::AID_KEYPLE_PREFIX);
    cardSelector1->setFileOccurrence(FileOccurrence::NEXT);

    /*
     * Prepare the selection by adding the created generic selection to the card
     * selection scenario.
     */
    cardSelectionManager->prepareSelection(
        cardSelector2, genericCardSelectionExtension);

    /* Close the channel after the selection */
    cardSelectionManager->prepareReleaseChannel();

    std::shared_ptr<CardSelectionResult> cardSelectionsResult
        = cardSelectionManager->processCardSelectionScenario(cardReader);

    /* Log the result */
    for (const auto& entry : cardSelectionsResult->getSmartCards()) {
        std::shared_ptr<IsoSmartCard> smartCard(
            std::dynamic_pointer_cast<IsoSmartCard>(entry.second));
        const std::string& powerOnData = smartCard->getPowerOnData();
        const std::string selectApplicationResponse
            = HexUtil::toHex(smartCard->getSelectApplicationResponse());
        const std::string selectionIsActive
            = smartCard == cardSelectionsResult->getActiveSmartCard() ? "true"
                                                                      : "false";

        logger->info(
            "Selection status for selection (indexed %): \n"
            "\t\tActive smart card: %\n"
            "\t\tpower-on data: %\n"
            "\t\tSelect Application response: %\n",
            entry.first,
            selectionIsActive,
            powerOnData,
            selectApplicationResponse);
    }

    logger->info("= #### End of the generic card processing\n");

    return 0;
}

int
main() {
    try {
        return runExample();

    } catch (const std::exception& e) {
        logger->error("Example terminated on exception: %\n", e.what());
        return 1;
    }
}
