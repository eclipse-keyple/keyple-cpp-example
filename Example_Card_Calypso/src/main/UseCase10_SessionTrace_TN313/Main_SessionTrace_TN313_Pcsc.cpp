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
#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keyple/plugin/pcsc/PcscSupportedContactProtocol.hpp"
#include "keyple/plugin/pcsc/PcscSupportedContactlessProtocol.hpp"
#include "keypop/calypso/card/CalypsoCardApiFactory.hpp"
#include "keypop/calypso/card/WriteAccessLevel.hpp"
#include "keypop/calypso/card/card/CalypsoCardSelectionExtension.hpp"
#include "keypop/calypso/card/transaction/SymmetricCryptoSecuritySetting.hpp"
#include "keypop/calypso/crypto/legacysam/LegacySamApiFactory.hpp"
#include "keypop/calypso/crypto/legacysam/sam/LegacySam.hpp"
#include "keypop/reader/CardReader.hpp"
#include "keypop/reader/ObservableCardReader.hpp"
#include "keypop/reader/ReaderApiFactory.hpp"
#include "keypop/reader/selection/CardSelectionManager.hpp"
#include "keypop/reader/selection/CardSelectionResult.hpp"
#include "keypop/reader/selection/IsoCardSelector.hpp"

#include "../common/ConfigurationUtil.hpp"
#include "CardReaderObserver.hpp"

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
using keyple::plugin::pcsc::PcscPluginFactoryBuilder;
using keyple::plugin::pcsc::PcscReader;
using keyple::plugin::pcsc::PcscSupportedContactlessProtocol;
using keyple::plugin::pcsc::PcscSupportedContactProtocol;
using keypop::calypso::card::CalypsoCardApiFactory;
using keypop::calypso::card::WriteAccessLevel;
using keypop::calypso::card::card::CalypsoCardSelectionExtension;
using keypop::calypso::card::transaction::SymmetricCryptoSecuritySetting;
using keypop::calypso::crypto::legacysam::LegacySamApiFactory;
using keypop::calypso::crypto::legacysam::sam::LegacySam;
using keypop::reader::CardReader;
using keypop::reader::ObservableCardReader;
using keypop::reader::ReaderApiFactory;
using keypop::reader::selection::CardSelectionManager;
using keypop::reader::selection::CardSelectionResult;
using keypop::reader::selection::IsoCardSelector;

/**
 * Handles the execution of Calypso Secure Session Trace as defined in
 * Technical Note #313 (PC/SC) using an observable reader and Calypso card
 * extension service.
 */
class Main_SessionTrace_TN313_Pcsc { };
static std::unique_ptr<Logger> logger
    = LoggerFactory::getLogger(typeid(Main_SessionTrace_TN313_Pcsc));

/* A regular expression for matching common contactless card readers. Adapt as
 * needed. */
static const std::string CARD_READER_NAME_REGEX
    = ".*ASK LoGO.*|.*Contactless.*";
/* A regular expression for matching common SAM readers. Adapt as needed. */
static const std::string SAM_READER_NAME_REGEX = ".*Identive.*|.*HID.*|.*SAM.*";
static const std::string ISO_CARD_PROTOCOL = "ISO_14443_4_CARD";
static const std::string SAM_PROTOCOL = "ISO_7816_3_T0";
static std::string cardReaderRegex = CARD_READER_NAME_REGEX;
static std::string samReaderRegex = SAM_READER_NAME_REGEX;

/** AID: Keyple test kit profile 1, Application 2 */
static const std::string AID = "315449432E49434131";

static std::string cardAid = AID;

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
 * Displays the expected options and exits.
 */
static void
displayUsageAndExit() {
    std::cout << "Available options:" << std::endl;
    std::cout << " -d, --default                  use default values (is "
                 "equivalent to -a=\""
              << AID << "\" -c=\"" << CARD_READER_NAME_REGEX << "\" -s=\""
              << SAM_READER_NAME_REGEX << "\")" << std::endl;
    std::cout << " -a, --aid=\"APPLICATION_AID\"    between 5 and 16 hex "
                 "bytes (e.g. \"315449432E49434131\")"
              << std::endl;
    std::cout << " -c, --card=\"CARD_READER_REGEX\" regular expression "
                 "matching the card reader name (e.g. \"ASK Logo.*\")"
              << std::endl;
    std::cout << " -s, --sam=\"SAM_READER_REGEX\"   regular expression "
                 "matching the SAM reader name (e.g. \"HID.*\")"
              << std::endl;
    std::cout << "PC/SC protocol is set to `\"ANY\" ('*') for the SAM "
                 "reader, \"T1\" ('T=1') for the card reader."
              << std::endl;

    exit(1);
}

/**
 * Splits a "key=value" argument into its two parts, handling additional
 * options such as the AID, the card reader regex and the SAM reader regex.
 */
static void
parseAdditionalArguments(const std::string& arg) {
    const std::string::size_type pos = arg.find('=');
    if (pos == std::string::npos) {
        displayUsageAndExit();
        return;
    }

    const std::string argKey = arg.substr(0, pos);
    const std::string argValue = arg.substr(pos + 1);

    if (argKey == "-a" || argKey == "--aid") {
        if (argValue.length() < 10 || argValue.length() > 32
            || !HexUtil::isValid(argValue)) {
            std::cout << "Invalid AID" << std::endl;
            displayUsageAndExit();
            return;
        }
        cardAid = argValue;
    } else if (argKey == "-c" || argKey == "--card") {
        cardReaderRegex = argValue;
    } else if (argKey == "-s" || argKey == "--sam") {
        samReaderRegex = argValue;
    } else {
        displayUsageAndExit();
    }
}

/**
 * Analyses the command line and sets the specified parameters.
 */
static void
parseCommandLine(int argc, char** argv) {
    if (argc == 1) {
        displayUsageAndExit();
        return;
    }

    for (int i = 1; i < argc; i++) {
        const std::string arg(argv[i]);
        if (arg == "-d" || arg == "--default") {
            break;
        }

        parseAdditionalArguments(arg);
    }
}

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
        PcscSupportedContactlessProtocol::ISO_14443_4.getName(),
        ISO_CARD_PROTOCOL);
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
        PcscSupportedContactProtocol::ISO_7816_3_T0.getName(),
        SAM_PROTOCOL);
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
    symmetricCryptoSecuritySetting
        ->assignDefaultKif(WriteAccessLevel::PERSONALIZATION, 0x21)
        .assignDefaultKif(WriteAccessLevel::LOAD, 0x27)
        .assignDefaultKif(WriteAccessLevel::DEBIT, 0x30)
        .enableRatificationMechanism();

    /* Optimization: preload the SAM challenge for the next transaction */
    symmetricCryptoSecuritySetting->initCryptoContextForNextTransaction();
}

int
main(int argc, char** argv) {
    parseCommandLine(argc, argv);

    logger->info(
        "= UseCase Calypso #10: session trace TN313 ==================\n");
    logger->info("Using parameters:\n");
    logger->info("  AID=%\n", cardAid);
    logger->info("  CARD_READER_REGEX=%\n", cardReaderRegex);
    logger->info("  SAM_READER_REGEX=%\n", samReaderRegex);

    /* Initialize the context */
    initKeypleService();
    initCalypsoCardExtensionService();
    initCardReader();
    initSamReader();
    initSecuritySetting();

    logger->info("Select application with AID = '%'\n", cardAid);

    /* Get the core card selection manager */
    std::shared_ptr<CardSelectionManager> cardSelectionManager(
        readerApiFactory->createCardSelectionManager());

    std::shared_ptr<IsoCardSelector> cardSelector(
        readerApiFactory->createIsoCardSelector());
    cardSelector->filterByDfName(cardAid);

    /*
     * Create a card selection using the Calypso card extension.
     * Select the card and read the record 1 of the file
     * ENVIRONMENT_AND_HOLDER. Prepare the selection by adding the created
     * Calypso selection to the card selection scenario.
     */
    std::unique_ptr<CalypsoCardSelectionExtension>
        calypsoCardSelectionExtension(
            calypsoCardApiFactory->createCalypsoCardSelectionExtension());
    calypsoCardSelectionExtension->acceptInvalidatedCard();
    cardSelectionManager->prepareSelection(
        cardSelector, std::move(calypsoCardSelectionExtension));

    /*
     * Schedule the selection scenario, request notification only if the card
     * matches the selection case.
     */
    auto observableCardReader
        = std::dynamic_pointer_cast<ObservableCardReader>(cardReader);
    cardSelectionManager->scheduleCardSelectionScenario(
        observableCardReader,
        ObservableCardReader::NotificationMode::MATCHED_ONLY);

    /* Create and add a card observer for this reader */
    auto cardReaderObserver = std::make_shared<CardReaderObserver>(
        plugin,
        observableCardReader,
        cardSelectionManager,
        symmetricCryptoSecuritySetting);

    observableCardReader->setReaderObservationExceptionHandler(
        cardReaderObserver);
    observableCardReader->addObserver(cardReaderObserver);
    observableCardReader->startCardDetection(
        ObservableCardReader::DetectionMode::REPEATING);

    logger->info("Wait for a card...\n");

    logger->info("Press ENTER to exit...\n");
    std::string line;
    std::getline(std::cin, line);
    logger->info("Exit in progress...\n");

    /* Unregister plugin */
    SmartCardServiceProvider::getService()->unregisterPlugin(plugin->getName());

    logger->info("Exit program\n");

    return 0;
}
