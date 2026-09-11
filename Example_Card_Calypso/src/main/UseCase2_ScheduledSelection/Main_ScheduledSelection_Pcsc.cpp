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

#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <utility>

#include "keyple/card/calypso/CalypsoExtensionService.hpp"
#include "keyple/core/service/Plugin.hpp"
#include "keyple/core/service/SmartCardService.hpp"
#include "keyple/core/service/SmartCardServiceProvider.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keyple/core/util/cpp/Thread.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keyple/plugin/pcsc/PcscSupportedContactlessProtocol.hpp"
#include "keypop/calypso/card/CalypsoCardApiFactory.hpp"
#include "keypop/calypso/card/card/CalypsoCardSelectionExtension.hpp"
#include "keypop/reader/CardReader.hpp"
#include "keypop/reader/ObservableCardReader.hpp"
#include "keypop/reader/ReaderApiFactory.hpp"
#include "keypop/reader/selection/CardSelectionManager.hpp"
#include "keypop/reader/selection/IsoCardSelector.hpp"

#include "../common/ConfigurationUtil.hpp"
#include "CardReaderObserver.hpp"

using keyple::card::calypso::CalypsoExtensionService;
using keyple::core::service::Plugin;
using keyple::core::service::SmartCardService;
using keyple::core::service::SmartCardServiceProvider;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keyple::core::util::cpp::Thread;
using keyple::plugin::pcsc::PcscPluginFactoryBuilder;
using keyple::plugin::pcsc::PcscReader;
using keyple::plugin::pcsc::PcscSupportedContactlessProtocol;
using keypop::calypso::card::CalypsoCardApiFactory;
using keypop::calypso::card::card::CalypsoCardSelectionExtension;
using keypop::reader::CardReader;
using keypop::reader::ObservableCardReader;
using keypop::reader::ReaderApiFactory;
using keypop::reader::selection::CardSelectionManager;
using keypop::reader::selection::IsoCardSelector;

/**
 * Handles the process of a Calypso card selection using the PC/SC plugin and
 * a scheduled selection scenario.
 *
 * <p>This class demonstrates the advanced selection of a Calypso card, where
 * the selection operations are prepared ahead of time. Using the card
 * selection manager and Calypso extension service, a scheduled scenario is
 * created and the reader is set to observation mode. Upon card insertion,
 * the prepared selection scenario is automatically executed, and the
 * observer is notified with the selection data collected.
 */
class Main_ScheduledSelection_Pcsc { };
static std::unique_ptr<Logger> logger
    = LoggerFactory::getLogger(typeid(Main_ScheduledSelection_Pcsc));

/** AID: Keyple test kit profile 1, Application 2 */
static const std::string AID = "A000000291FF9101";

/* File identifiers */
static const std::uint8_t SFI_ENVIRONMENT_AND_HOLDER = 0x07;

/* The plugin used to manage the reader. */
static std::shared_ptr<Plugin> plugin;
/* The reader used to communicate with the card. */
static std::shared_ptr<CardReader> cardReader;
/* The factory used to create the selection manager and card selectors. */
static std::shared_ptr<ReaderApiFactory> readerApiFactory;
/*
 * The Calypso factory used to create the selection extension and transaction
 * managers.
 */
static std::shared_ptr<CalypsoCardApiFactory> calypsoCardApiFactory;

/**
 * Initializes the Keyple service.
 *
 * <p>Gets an instance of the smart card service, registers the PC/SC plugin,
 * and prepares the reader API factory for use.
 */
static void
initKeypleService() {
    std::shared_ptr<SmartCardService> smartCardService(
        SmartCardServiceProvider::getService());
    plugin = smartCardService->registerPlugin(
        PcscPluginFactoryBuilder::builder()->build());
    readerApiFactory = smartCardService->getReaderApiFactory();
}

/**
 * Initializes the card reader with specific configurations.
 */
static void
initCardReader() {
    cardReader = ConfigurationUtil::getReader(
        plugin,
        ConfigurationUtil::CARD_READER_NAME_REGEX,
        true,
        PcscReader::IsoProtocol::T1,
        PcscReader::SharingMode::SHARED,
        PcscSupportedContactlessProtocol::ISO_14443_4.getName(),
        ConfigurationUtil::ISO_CARD_PROTOCOL);
}

/**
 * Initializes the Calypso card extension service.
 */
static void
initCalypsoCardExtensionService() {
    std::shared_ptr<CalypsoExtensionService> calypsoExtensionService(
        CalypsoExtensionService::getInstance());
    SmartCardServiceProvider::getService()->checkCardExtension(
        calypsoExtensionService);
    calypsoCardApiFactory = calypsoExtensionService->getCalypsoCardApiFactory();
}

static int
runExample() {
    logger->info(
        "= UseCase Generic #2: scheduled selection ==================\n");

    /* Initialize the context */
    initKeypleService();
    initCardReader();
    initCalypsoCardExtensionService();

    logger->info("= #### Select application with AID = '%'\n", AID);

    /* Retrieve the core card selection manager from the Reader API factory */
    std::shared_ptr<CardSelectionManager> cardSelectionManager(
        readerApiFactory->createCardSelectionManager());

    /* Create an ISO card selector and set a filter by the specific AID */
    std::shared_ptr<IsoCardSelector> cardSelector(
        readerApiFactory->createIsoCardSelector());
    cardSelector->filterByDfName(AID);

    /*
     * Initialize a Calypso card selection extension read the record 1 of the
     * file ENVIRONMENT_AND_HOLDER.
     */
    std::unique_ptr<CalypsoCardSelectionExtension>
        calypsoCardSelectionExtension(
            calypsoCardApiFactory->createCalypsoCardSelectionExtension());
    calypsoCardSelectionExtension->acceptInvalidatedCard().prepareReadRecord(
        SFI_ENVIRONMENT_AND_HOLDER, 1);

    /*
     * Prepare the card selection scenario by associating the card selector
     * with the Calypso card selection extension.
     */
    cardSelectionManager->prepareSelection(
        cardSelector, std::move(calypsoCardSelectionExtension));

    /*
     * Schedule the card selection scenario with a configuration to notify only
     * if a card matches the selection criteria.
     */
    auto observableCardReader
        = std::dynamic_pointer_cast<ObservableCardReader>(cardReader);
    cardSelectionManager->scheduleCardSelectionScenario(
        observableCardReader,
        ObservableCardReader::NotificationMode::MATCHED_ONLY);

    /*
     * Establish a card reader observer to manage card reader events and
     * errors and initiate the card detection process.
     */
    auto cardReaderObserver = std::make_shared<CardReaderObserver>(
        observableCardReader, cardSelectionManager);
    observableCardReader->setReaderObservationExceptionHandler(
        cardReaderObserver);
    observableCardReader->addObserver(cardReaderObserver);
    observableCardReader->startCardDetection(
        ObservableCardReader::DetectionMode::REPEATING);

    logger->info(
        "= #### Wait for a card. The default AID based selection to be "
        "processed as soon as the card is detected.\n");

    /* The program will remain active indefinitely until interrupted (CTRL-C to
     * exit) */
    while (true) {
        Thread::sleep(100);
    }
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
