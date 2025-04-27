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

#include <memory>

#include "CardReaderObserver.hpp"
#include "common/ConfigurationUtil.hpp"

#include "keyple/card/generic/GenericExtensionService.hpp"
#include "keyple/core/service/Plugin.hpp"
#include "keyple/core/service/SmartCardService.hpp"
#include "keyple/core/service/SmartCardServiceProvider.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keyple/plugin/pcsc/PcscSupportedContactlessProtocol.hpp"
#include "keypop/reader/ConfigurableCardReader.hpp"
#include "keypop/reader/ObservableCardReader.hpp"
#include "keypop/reader/ReaderApiFactory.hpp"
#include "keypop/reader/selection/CardSelectionManager.hpp"
#include "keypop/reader/selection/IsoCardSelector.hpp"

using keyple::card::generic::GenericExtensionService;
using keyple::core::service::Plugin;
using keyple::core::service::SmartCardService;
using keyple::core::service::SmartCardServiceProvider;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keyple::plugin::pcsc::PcscPluginFactoryBuilder;
using keyple::plugin::pcsc::PcscReader;
using keyple::plugin::pcsc::PcscSupportedContactlessProtocol;
using keypop::reader::ConfigurableCardReader;
using keypop::reader::ObservableCardReader;
using keypop::reader::ReaderApiFactory;
using keypop::reader::selection::CardSelectionManager;
using keypop::reader::selection::IsoCardSelector;

/**
 * <h1>Use Case Generic 4 – Scheduled Selection (PC/SC)</h1>
 *
 * <p>We present here a selection of ISO-14443-4 cards including the
 * transmission of a "select application" APDU targeting EMV banking cards
 * (AID PPSE). Any contactless EMV card should lead to
 * a "selected" state, any card with another DF Name should be ignored.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Schedule a selection scenario over an observable reader to target a
 *       specific card (here a EMV contactless card).
 *   <li>Start the observation and wait for a card.
 *   <li>Within the reader event handler:
 *       <ul>
 *         <li>Output collected smart card data (FCI and power-on data).
 *         <li>Close the physical channel.
 *       </ul>
 * </ul>
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 */
class Main_ScheduledSelection_Pcsc { };
const std::unique_ptr<Logger> logger
    = LoggerFactory::getLogger(typeid(Main_ScheduledSelection_Pcsc));

int
main() {
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
    std::shared_ptr<GenericExtensionService> genericCardService(
        GenericExtensionService::getInstance());

    /*
     * Verify that the extension's API level is consistent with the current
     * service.
     */
    smartCardService->checkCardExtension(genericCardService);

    /* Get the contactless reader whose name matches the provided regex */
    auto observableCardReader = std::dynamic_pointer_cast<ObservableCardReader>(
        plugin->findReader(ConfigurationUtil::CONTACTLESS_READER_NAME_REGEX));

    /*
     * Configure the reader with parameters suitable for contactless operations.
     */
    std::dynamic_pointer_cast<PcscReader>(
        plugin->getReaderExtension(
            typeid(PcscReader), observableCardReader->getName()))
        ->setContactless(true)
        .setIsoProtocol(PcscReader::IsoProtocol::T1)
        .setSharingMode(PcscReader::SharingMode::SHARED);

    std::dynamic_pointer_cast<ConfigurableCardReader>(observableCardReader)
        ->activateProtocol(
            PcscSupportedContactlessProtocol::ISO_14443_4.getName(),
            ConfigurationUtil::ISO_CARD_PROTOCOL);

    logger->info("=============== "
                 "UseCase Generic #4: scheduled AID based selection "
                 "===============\n");

    logger->info(
        "= #### Select application with AID = '%'\n",
        ConfigurationUtil::AID_EMV_PPSE);

    std::shared_ptr<ReaderApiFactory> readerApiFactory(
        smartCardService->getReaderApiFactory());

    /* Get the core card selection manager */
    std::shared_ptr<CardSelectionManager> cardSelectionManager
        = readerApiFactory->createCardSelectionManager();

    /* Create a card selection using the generic card extension */
    std::shared_ptr<IsoCardSelector> cardSelector(
        readerApiFactory->createIsoCardSelector());
    cardSelector->filterByCardProtocol(ConfigurationUtil::ISO_CARD_PROTOCOL);
    cardSelector->filterByDfName(ConfigurationUtil::AID_EMV_PPSE);

    /*
     * Prepare the selection by adding the created generic selection to the card
     * selection scenario
     */
    cardSelectionManager->prepareSelection(
        cardSelector,
        GenericExtensionService::getInstance()
            ->createGenericCardSelectionExtension());

    /* Schedule the selection scenario */
    cardSelectionManager->scheduleCardSelectionScenario(
        observableCardReader,
        ObservableCardReader::NotificationMode::MATCHED_ONLY);

    /* Create and add an observer */
    auto cardReaderObserver = std::make_shared<CardReaderObserver>(
        observableCardReader, cardSelectionManager);
    observableCardReader->setReaderObservationExceptionHandler(
        cardReaderObserver);
    observableCardReader->addObserver(cardReaderObserver);
    observableCardReader->startCardDetection(
        ObservableCardReader::DetectionMode::REPEATING);

    logger->info(
        "= #### Wait for a card. The AID based selection scenario will be "
        "processed as soon as a card is detected\n");

    while (true);
}
