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

#include "keyple/card/generic/ChannelControl.hpp"
#include "keyple/card/generic/GenericExtensionService.hpp"
#include "keyple/core/service/Plugin.hpp"
#include "keyple/core/service/SmartCardService.hpp"
#include "keyple/core/service/SmartCardServiceProvider.hpp"
#include "keyple/core/util/HexUtil.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keyple/core/util/cpp/exception/IllegalStateException.hpp"
#include "keyple/plugin/pcsc/PcscCardCommunicationProtocol.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keypop/reader/CardReader.hpp"
#include "keypop/reader/ConfigurableCardReader.hpp"
#include "keypop/reader/ReaderApiFactory.hpp"
#include "keypop/reader/selection/CardSelectionManager.hpp"
#include "keypop/reader/selection/CardSelectionResult.hpp"
#include "keypop/reader/selection/IsoCardSelector.hpp"
#include "keypop/reader/selection/spi/SmartCard.hpp"

using keyple::card::generic::ChannelControl;
using keyple::card::generic::GenericExtensionService;
using keyple::core::service::Plugin;
using keyple::core::service::SmartCardService;
using keyple::core::service::SmartCardServiceProvider;
using keyple::core::util::HexUtil;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keyple::core::util::cpp::exception::IllegalStateException;
using keyple::plugin::pcsc::PcscCardCommunicationProtocol;
using keyple::plugin::pcsc::PcscPluginFactoryBuilder;
using keyple::plugin::pcsc::PcscReader;
using keypop::reader::CardReader;
using keypop::reader::ConfigurableCardReader;
using keypop::reader::ReaderApiFactory;
using keypop::reader::selection::CardSelectionManager;
using keypop::reader::selection::CardSelectionResult;
using keypop::reader::selection::IsoCardSelector;
using keypop::reader::selection::spi::SmartCard;

/**
 * <h1>Use Case Generic 3 – AID Based Selection (PC/SC)</h1>
 *
 * <p>We present here a selection of cards including the transmission of a
 * "select application" APDU targeting a specific DF Name. Any card with an
 * application whose DF Name starts with the provided AID should lead to a
 * "selected" state, any card with another DF Name should be ignored.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Check if a ISO 14443-4 card is in the reader, select a card with the
 *       specified AID (here the EMV PPSE AID).
 *   <li>Run a selection scenario with the DF Name filter.
 *   <li>Output the collected smart card data (power-on data).
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_AidBasedSelection_Pcsc { };
const std::unique_ptr<Logger> logger
    = LoggerFactory::getLogger(typeid(Main_AidBasedSelection_Pcsc));

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
    std::shared_ptr<GenericExtensionService> genericCardService(
        GenericExtensionService::getInstance());

    /*
     * Verify that the extension's API level is consistent with the current
     * service
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
            PcscCardCommunicationProtocol::ISO_14443_4.getName(),
            ConfigurationUtil::ISO_CARD_PROTOCOL);

    logger->info("=============== "
                 "UseCase Generic #3: AID based card selection "
                 "==================\n");

    /* Check if a card is present in the reader */
    if (!cardReader->isCardPresent()) {
        logger->error("No card is present in the reader\n");
        return 0;
    }

    logger->info(
        "= #### Select the card if its DF Name matches '%'\n",
        ConfigurationUtil::AID_EMV_PPSE);

    auto readerApiFactory(smartCardService->getReaderApiFactory());

    /* Get the core card selection manager */
    std::shared_ptr<CardSelectionManager> cardSelectionManager
        = readerApiFactory->createCardSelectionManager();

    /*
     * Create a card selection using the generic card extension and specifying a
     * DfName filter.
     */
    std::shared_ptr<IsoCardSelector> cardSelector(
        readerApiFactory->createIsoCardSelector());
    cardSelector->filterByDfName(ConfigurationUtil::AID_EMV_PPSE);

    /*
     * Prepare the selection by adding the created generic selection to the card
     * selection scenario.
     */
    auto genericCardSelectionExtension(
        GenericExtensionService::getInstance()
            ->createGenericCardSelectionExtension());
    cardSelectionManager->prepareSelection(
        cardSelector, genericCardSelectionExtension);

    /* Actual card communication: run the selection scenario */
    std::shared_ptr<CardSelectionResult> selectionResult
        = cardSelectionManager->processCardSelectionScenario(cardReader);

    /* Check the selection result */
    if (selectionResult->getActiveSmartCard() == nullptr) {
        logger->error("The selection of the card failed\n");
        return 0;
    }

    /* Get the SmartCard resulting of the selection */
    std::shared_ptr<SmartCard> smartCard
        = selectionResult->getActiveSmartCard();

    logger->info("= SmartCard = %\n", smartCard);

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
