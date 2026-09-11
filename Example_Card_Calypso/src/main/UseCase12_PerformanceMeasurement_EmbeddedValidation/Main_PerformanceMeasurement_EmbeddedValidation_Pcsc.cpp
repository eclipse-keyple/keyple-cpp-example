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

#include <chrono>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "keyple/card/calypso/CalypsoExtensionService.hpp"
#include "keyple/card/calypso/crypto/legacysam/LegacySamExtensionService.hpp"
#include "keyple/card/calypso/crypto/legacysam/LegacySamUtil.hpp"
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
#include "keypop/calypso/card/CalypsoCardApiFactory.hpp"
#include "keypop/calypso/card/WriteAccessLevel.hpp"
#include "keypop/calypso/card/card/CalypsoCard.hpp"
#include "keypop/calypso/card/card/CalypsoCardSelectionExtension.hpp"
#include "keypop/calypso/card/card/ElementaryFile.hpp"
#include "keypop/calypso/card/card/FileData.hpp"
#include "keypop/calypso/card/cpp/SecureRegularModeTransactionManagerBase.hpp"
#include "keypop/calypso/card/transaction/SecureSymmetricCryptoTransactionManager.hpp"
#include "keypop/calypso/card/transaction/SymmetricCryptoSecuritySetting.hpp"
#include "keypop/calypso/crypto/legacysam/LegacySamApiFactory.hpp"
#include "keypop/calypso/crypto/legacysam/sam/LegacySam.hpp"
#include "keypop/reader/CardReader.hpp"
#include "keypop/reader/ChannelControl.hpp"
#include "keypop/reader/ReaderApiFactory.hpp"
#include "keypop/reader/selection/CardSelectionManager.hpp"
#include "keypop/reader/selection/CardSelectionResult.hpp"
#include "keypop/reader/selection/IsoCardSelector.hpp"
#include "keypop/reader/selection/spi/SmartCard.hpp"

#include "../common/ConfigurationUtil.hpp"

using keyple::card::calypso::CalypsoExtensionService;
using keyple::card::calypso::crypto::legacysam::LegacySamExtensionService;
using keyple::card::calypso::crypto::legacysam::LegacySamUtil;
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
using keypop::calypso::card::CalypsoCardApiFactory;
using keypop::calypso::card::WriteAccessLevel;
using keypop::calypso::card::card::CalypsoCard;
using keypop::calypso::card::card::CalypsoCardSelectionExtension;
using keypop::calypso::card::cpp::SecureRegularModeTransactionManagerBase;
using keypop::calypso::card::transaction::
    SecureSymmetricCryptoTransactionManager;
using keypop::calypso::card::transaction::SymmetricCryptoSecuritySetting;
using keypop::calypso::crypto::legacysam::LegacySamApiFactory;
using keypop::calypso::crypto::legacysam::sam::LegacySam;
using keypop::reader::CardReader;
using keypop::reader::ChannelControl;
using keypop::reader::ReaderApiFactory;
using keypop::reader::selection::CardSelectionManager;
using keypop::reader::selection::CardSelectionResult;
using keypop::reader::selection::IsoCardSelector;
using keypop::reader::selection::spi::SmartCard;

/**
 * Use Case Calypso 12 - Performance measurement: embedded validation (PC/SC)
 *
 * <p>This code is dedicated to performance measurement for an embedded
 * validation type transaction.
 */
class Main_PerformanceMeasurement_EmbeddedValidation_Pcsc { };
static std::unique_ptr<Logger> logger = LoggerFactory::getLogger(
    typeid(Main_PerformanceMeasurement_EmbeddedValidation_Pcsc));

static const std::string ANSI_RESET = "\033[0m";
static const std::string ANSI_RED = "\033[31m";
static const std::string ANSI_GREEN = "\033[32m";
static const std::string ANSI_YELLOW = "\033[33m";

/* Operating parameters. Edit as needed to match your setup. */
static const std::string cardReaderRegex
    = ConfigurationUtil::CARD_READER_NAME_REGEX;
static const std::string samReaderRegex
    = ConfigurationUtil::SAM_READER_NAME_REGEX;

/** AID: Keyple test kit profile 1, Application 2 */
static const std::string cardAid = "A000000291FF9101";

static const int counterDecrement = 1;
static const std::vector<std::uint8_t> newEventRecord(
    HexUtil::toByteArray(
        "8013C8EC55667788112233445566778811223344556677881122334455"));

static const std::uint8_t SFI_ENVIRONMENT_AND_HOLDER = 0x07;
static const std::uint8_t SFI_EVENT_LOG = 0x08;
static const std::uint8_t SFI_CONTRACT_LIST = 0x1E;
static const std::uint8_t SFI_CONTRACTS = 0x09;
static const std::uint8_t SFI_COUNTERS = 0x19;
static const int RECORD_SIZE = 29;

/* The plugin used to manage the readers. */
static std::shared_ptr<Plugin> plugin;
/* The reader used to communicate with the card. */
static std::shared_ptr<CardReader> cardReader;
/* The reader used to communicate with the SAM. */
static std::shared_ptr<CardReader> samReader;
/* The factory used to create the selection manager and card selectors. */
static std::shared_ptr<ReaderApiFactory> readerApiFactory;
/*
 * The Calypso factory used to create the selection extension and transaction
 * managers.
 */
static std::shared_ptr<CalypsoCardApiFactory> calypsoCardApiFactory;
/* The security settings for the card transaction. */
static std::shared_ptr<SymmetricCryptoSecuritySetting>
    symmetricCryptoSecuritySetting;

/**
 * Initializes the Keyple service.
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
        cardReaderRegex,
        true,
        PcscReader::IsoProtocol::T1,
        PcscReader::SharingMode::SHARED,
        PcscCardCommunicationProtocol::ISO_14443_4.getName(),
        ConfigurationUtil::ISO_CARD_PROTOCOL);
}

/**
 * Initializes the SAM reader with specific configurations.
 */
static void
initSamReader() {
    samReader = ConfigurationUtil::getReader(
        plugin,
        samReaderRegex,
        false,
        PcscReader::IsoProtocol::ANY,
        PcscReader::SharingMode::SHARED,
        PcscCardCommunicationProtocol::ISO_7816_3.getName(),
        ConfigurationUtil::SAM_PROTOCOL);
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

/**
 * Selects the SAM C1 for the transaction.
 */
static std::shared_ptr<LegacySam>
selectSam(std::shared_ptr<CardReader> reader) {
    std::shared_ptr<CardSelectionManager> samSelectionManager(
        readerApiFactory->createCardSelectionManager());

    std::shared_ptr<IsoCardSelector> cardSelector(
        readerApiFactory->createIsoCardSelector());
    cardSelector->filterByPowerOnData(
        LegacySamUtil::buildPowerOnDataFilter(
            LegacySam::ProductType::SAM_C1, ""));

    std::shared_ptr<LegacySamApiFactory> legacySamApiFactory(
        LegacySamExtensionService::getInstance()->getLegacySamApiFactory());

    samSelectionManager->prepareSelection(
        cardSelector, legacySamApiFactory->createLegacySamSelectionExtension());

    const std::shared_ptr<CardSelectionResult> samSelectionResult(
        samSelectionManager->processCardSelectionScenario(reader));

    if (samSelectionResult->getActiveSmartCard() == nullptr) {
        throw IllegalStateException("The selection of the SAM failed.");
    }

    return std::dynamic_pointer_cast<LegacySam>(
        samSelectionResult->getActiveSmartCard());
}

/**
 * Initializes the security settings for the transaction.
 */
static void
initSecuritySetting() {
    std::shared_ptr<LegacySam> sam(selectSam(samReader));

    symmetricCryptoSecuritySetting
        = calypsoCardApiFactory->createSymmetricCryptoSecuritySetting(
            LegacySamExtensionService::getInstance()
                ->getLegacySamApiFactory()
                ->createSymmetricCryptoCardTransactionManagerFactory(
                    samReader, sam));
    symmetricCryptoSecuritySetting->enableRatificationMechanism();

    /* Optimization: preload the SAM challenge for the next transaction */
    symmetricCryptoSecuritySetting->initCryptoContextForNextTransaction();
}

/**
 * Selects the Calypso card for the transaction based on the specified
 * Application Identifier (AID).
 */
static std::shared_ptr<CalypsoCard>
selectCard(std::shared_ptr<CardReader> reader, const std::string& aid) {
    std::shared_ptr<CardSelectionManager> cardSelectionManager(
        readerApiFactory->createCardSelectionManager());
    std::shared_ptr<IsoCardSelector> cardSelector(
        readerApiFactory->createIsoCardSelector());
    cardSelector->filterByDfName(aid);

    std::unique_ptr<CalypsoCardSelectionExtension>
        calypsoCardSelectionExtension(
            calypsoCardApiFactory->createCalypsoCardSelectionExtension());
    calypsoCardSelectionExtension->acceptInvalidatedCard();
    cardSelectionManager->prepareSelection(
        cardSelector, std::move(calypsoCardSelectionExtension));

    const std::shared_ptr<CardSelectionResult> selectionResult(
        cardSelectionManager->processCardSelectionScenario(reader));

    if (selectionResult->getActiveSmartCard() == nullptr) {
        return nullptr;
    }

    const std::shared_ptr<SmartCard> card(
        selectionResult->getActiveSmartCard());

    return std::dynamic_pointer_cast<CalypsoCard>(card);
}

static int
runExample() {
    logger->info(
        "%=============== Performance measurement: validation transaction "
        "===============\n",
        ANSI_GREEN);
    logger->info("Using parameters:\n");
    logger->info("  CARD_READER_REGEX=%\n", cardReaderRegex);
    logger->info("  SAM_READER_REGEX=%\n", samReaderRegex);
    logger->info("  AID=%\n", cardAid);
    logger->info("  Counter decrement=%\n", counterDecrement);
    logger->info("%\n", ANSI_RESET);

    /* Initialize the context */
    initKeypleService();
    initCalypsoCardExtensionService();
    initCardReader();
    initSamReader();
    initSecuritySetting();

    while (true) {
        std::cout << std::endl
                  << ANSI_YELLOW
                  << "########################################################"
                  << ANSI_RESET << std::endl;
        std::cout << ANSI_YELLOW
                  << "## Press ENTER when the card is in the reader's field ##"
                  << ANSI_RESET << std::endl;
        std::cout << ANSI_YELLOW
                  << "## (or press 'q' + ENTER to exit)                     ##"
                  << ANSI_RESET << std::endl;
        std::cout << ANSI_YELLOW
                  << "########################################################"
                  << ANSI_RESET << std::endl;

        std::string input;
        std::getline(std::cin, input);

        if (input.find('q') != std::string::npos
            || input.find('Q') != std::string::npos) {
            break;
        }

        if (cardReader->isCardPresent()) {
            try {
                logger->info("Starting validation transaction...\n");
                logger->info("Select application with AID = '%'\n", cardAid);

                /* Read the current time used later to compute the transaction
                 * time */
                const auto timeStamp = std::chrono::steady_clock::now();

                std::shared_ptr<CalypsoCard> calypsoCard(
                    selectCard(cardReader, cardAid));
                if (calypsoCard == nullptr) {
                    throw IllegalStateException("Card selection failed!");
                }

                /*
                 * Create a transaction manager, open a Secure Session, read
                 * Environment and Event Log.
                 * Specifying expected response lengths in read commands
                 * serves as a protective measure for legacy cards.
                 *
                 * The keypop API declares
                 * createSecureRegularModeTransactionManager() as returning a
                 * SecureRegularModeTransactionManagerBase, which does not
                 * itself expose prepareOpenSecureSession() (it's only
                 * declared on SecureSymmetricCryptoTransactionManager<T>, a
                 * sibling interface implemented by the same concrete
                 * object). A downcast is required to reach it.
                 */
                std::unique_ptr<SecureRegularModeTransactionManagerBase>
                    cardTransactionManagerBase(
                        calypsoCardApiFactory
                            ->createSecureRegularModeTransactionManager(
                                cardReader,
                                calypsoCard,
                                symmetricCryptoSecuritySetting));

                auto cardTransactionManager
                    = dynamic_cast<SecureSymmetricCryptoTransactionManager<
                        SecureRegularModeTransactionManagerBase>*>(
                        cardTransactionManagerBase.get());

                cardTransactionManager
                    ->prepareOpenSecureSession(WriteAccessLevel::DEBIT)
                    .prepareReadRecords(
                        SFI_ENVIRONMENT_AND_HOLDER, 1, 1, RECORD_SIZE)
                    .prepareReadRecords(SFI_EVENT_LOG, 1, 1, RECORD_SIZE)
                    .processCommands(ChannelControl::KEEP_OPEN);

                /* TODO Place here the analysis of the context and the last
                 * event log */

                /*
                 * Read the contract list.
                 * Specifying expected response lengths in read commands
                 * serves as a protective measure for legacy cards.
                 */
                cardTransactionManagerBase
                    ->prepareReadRecords(SFI_CONTRACT_LIST, 1, 1, RECORD_SIZE)
                    .processCommands(ChannelControl::KEEP_OPEN);

                /* TODO Place here the analysis of the contract list */

                /*
                 * Read the elected contract.
                 * Specifying expected response lengths in read commands
                 * serves as a protective measure for legacy cards.
                 */
                cardTransactionManagerBase
                    ->prepareReadRecords(SFI_CONTRACTS, 1, 1, RECORD_SIZE)
                    .processCommands(ChannelControl::KEEP_OPEN);

                /* TODO Place here the analysis of the contract */

                /* Read the contract counter */
                cardTransactionManagerBase->prepareReadCounter(SFI_COUNTERS, 1)
                    .processCommands(ChannelControl::KEEP_OPEN);

                /* TODO Place here the preparation of the card's content update
                 */

                /* Add an event record and close the Secure Session */
                cardTransactionManagerBase
                    ->prepareDecreaseCounter(SFI_COUNTERS, 1, counterDecrement)
                    .prepareAppendRecord(SFI_EVENT_LOG, newEventRecord)
                    .prepareCloseSecureSession()
                    .processCommands(ChannelControl::KEEP_OPEN);

                /* Display transaction time */
                const auto elapsedMs
                    = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now() - timeStamp)
                          .count();
                logger->info(
                    "%Transaction succeeded. Execution time: % ms%\n",
                    ANSI_GREEN,
                    elapsedMs,
                    ANSI_RESET);

                /* Optimization: preload the SAM challenge for the next
                 * transaction */
                symmetricCryptoSecuritySetting
                    ->initCryptoContextForNextTransaction();
            } catch (const std::exception& e) {
                logger->info(
                    "%Transaction failed with exception: % %\n",
                    ANSI_RED,
                    e.what(),
                    ANSI_RESET);
            }
        } else {
            logger->info("%No card detected%\n", ANSI_RED, ANSI_RESET);
        }
    }

    logger->info("Exiting the program on user's request.\n");

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
