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
 * <h1>Use Case Generic 5 – Sequential selections based on an AID prefix
 * (PC/SC)</h1>
 *
 * <p>We demonstrate here the selection of two applications in a single card,
 * with both applications selected sequentially using the same AID and the
 * "FIRST" and "NEXT" navigation options.<br>
 * The result of the first selection is available to the application before the
 * second selection is executed.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Check if a ISO 14443-4 card is in the reader, select a card (here a
 *       card having two applications whose DF Names are prefixed by a specific
 *       AID [see AID_KEYPLE_PREFIX]).
 *   <li>Run an AID based application selection scenario (first occurrence).
 *   <li>Output collected smart card data (FCI and power-on data).
 *   <li>Run an AID based application selection scenario (next occurrence).
 *   <li>Output collected smart card data (FCI and power-on data).
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_SequentialMultiSelection_Pcsc { };
const std::unique_ptr<Logger> logger
    = LoggerFactory::getLogger(typeid(Main_SequentialMultiSelection_Pcsc));

/**
 * Performs the selection for the provided reader and logs its result.
 *
 * <p>The card selection manager must have been previously assigned a
 * selection case.
 *
 * @param cardReader The reader.
 * @param cardSelectionsService The card selection manager.
 * @param index An int indicating the selection rank.
 */
static void
doAndAnalyseSelection(
    std::shared_ptr<CardReader> cardReader,
    std::shared_ptr<CardSelectionManager> cardSelectionsService,
    const int index) {
    std::shared_ptr<CardSelectionResult> cardSelectionsResult(
        cardSelectionsService->processCardSelectionScenario(cardReader));

    if (cardSelectionsResult->getActiveSmartCard() != nullptr) {
        std::shared_ptr<IsoSmartCard> isoSmartCard(
            std::dynamic_pointer_cast<IsoSmartCard>(
                cardSelectionsResult->getActiveSmartCard()));

        logger->info("The card matched the selection %\n", index);

        const std::string& powerOnData = isoSmartCard->getPowerOnData();
        const std::string selectApplicationResponse
            = HexUtil::toHex(isoSmartCard->getSelectApplicationResponse());

        logger->info(
            "Selection status for case %: \n"
            "\t\tpower-on data: %\n"
            "\t\tSelect Application response: %\n",
            index,
            powerOnData,
            selectApplicationResponse);
    } else {
        logger->info("The selection did not match for case %\n", index);
    }
}

static int
runExample() {
    logger->setLoggerLevel(Logger::Level::logTrace);

    /* Get the instance of the SmartCardService (singleton pattern) */
    std::shared_ptr<SmartCardService> smartCardService(
        SmartCardServiceProvider::getService());

    /*
     * Register the PcscPlugin with the SmartCardService, get the corresponding
     * generic plugin in return.
     */
    std::shared_ptr<Plugin> plugin = smartCardService->registerPlugin(
        PcscPluginFactoryBuilder::builder()->build());

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
                 "UseCase Generic #5: sequential selections based on an AID "
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

    /*
     * AID based selection: get the first application occurrence matching the
     * AID, keep the physical channel open
     */
    std::shared_ptr<IsoCardSelector> cardSelector(
        readerApiFactory->createIsoCardSelector());
    cardSelector->filterByDfName(ConfigurationUtil::AID_KEYPLE_PREFIX);
    cardSelector->setFileOccurrence(FileOccurrence::FIRST);

    std::shared_ptr<GenericCardSelectionExtension> genericCardSelectionExtension
        = GenericExtensionService::getInstance()
              ->createGenericCardSelectionExtension();

    /*
     * Prepare the selection by adding the created generic selection to the card
     * selection scenario.
     */
    cardSelectionManager->prepareSelection(
        cardSelector, genericCardSelectionExtension);

    /* Do the selection and display the result */
    doAndAnalyseSelection(cardReader, cardSelectionManager, 1);

    /*
     * New selection: get the next application occurrence matching the same AID,
     * Close the physical channel after.
     */
    cardSelector = readerApiFactory->createIsoCardSelector();
    cardSelector->filterByDfName(ConfigurationUtil::AID_KEYPLE_PREFIX);
    cardSelector->setFileOccurrence(FileOccurrence::NEXT);

    /*
     * Prepare the selection by adding the created generic selection to the card
     * selection scenario.
     */
    cardSelectionManager->prepareSelection(
        cardSelector, genericCardSelectionExtension);

    /* Close the channel after the selection */
    cardSelectionManager->prepareReleaseChannel();

    /* Do the selection and display the result */
    doAndAnalyseSelection(cardReader, cardSelectionManager, 2);

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
