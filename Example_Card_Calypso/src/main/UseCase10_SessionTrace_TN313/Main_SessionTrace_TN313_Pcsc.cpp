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

/* Keyple Cpp Example */
#include "CalypsoConstants.h"
#include "CardReaderObserver.h"
#include "ConfigurationUtil.h"

using namespace calypsonet::terminal::reader;
using namespace keyple::card::calypso;
using namespace keyple::core::service;
using namespace keyple::core::service::resource;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp;
using namespace keyple::core::util::cpp::exception;
using namespace keyple::core::util::protocol;
using namespace keyple::plugin::pcsc;

/**
 * Use Case Calypso 10 – Calypso Secure Session Trace - Technical Note #313 (PC/SC)
 *
 * <p>This an implementation of the Calypso Secure Session described the technical note #313
 * defining a typical usage of a Calypso card and allowing performances comparison.
 *
 * <p>Scenario:
 *
 * <ul>
 *   <li>Schedule a selection scenario over an observable reader to target a specific card (here a
 *       Calypso card characterized by its AID) and including the reading of a file record.
 *   <li>Attempts to select a Calypso SAM (C1) in the contact reader.
 *   <li>Start the observation and wait for a card insertion.
 *   <li>Within the reader event handler:
 *       <ul>
 *         <li>Do the TN313 transaction scenario.
 *         <li>Close the physical channel.
 *       </ul>
 * </ul>
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 */

class Main_SessionTrace_TN313_Pcsc{};
const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_SessionTrace_TN313_Pcsc));

static std::string cardReaderRegex = ConfigurationUtil::CARD_READER_NAME_REGEX;
static std::string samReaderRegex = ConfigurationUtil::SAM_READER_NAME_REGEX;
static std::string cardAid = CalypsoConstants::AID;
static bool isVerbose;

/**
 * Displays the expected options
 */
static void displayUsageAndExit()
{
    std::cout << "Available options:" << std::endl;
    std::cout << " -d, --default                  use default values (is equivalent to -a=" \
              << CalypsoConstants::AID <<  " -c=" << ConfigurationUtil::CARD_READER_NAME_REGEX \
              << " -s=" << ConfigurationUtil::SAM_READER_NAME_REGEX << ")" << std::endl;
    std::cout << " -a, --aid=\"APPLICATION_AID\"    between 5 and 16 hex bytes (e.g. \"315449432E" \
              << "49434131\")" << std::endl;
    std::cout << " -c, --card=\"CARD_READER_REGEX\" regular expression matching the card reader n" \
              << "ame (e.g. \"ASK Logo.*\")" << std::endl;
    std::cout << " -s, --sam=\"SAM_READER_REGEX\"   regular expression matching the SAM reader na" \
              << "me (e.g. \"HID.*\")" << std::endl;
    std::cout << " -v, --verbose                  set the log level to TRACE" << std::endl;
    std::cout << "PC/SC protocol is set to `\"ANY\" ('*') for the SAM reader, \"T1\" ('T=1') for " \
                 "the card reader." << std::endl;

    exit(-1);
}

/**
 * Analyses the command line and sets the specified parameters.
 *
 * @param args The command line arguments
 */
static void parseCommandLine(int argc, char **argv)
{
    /* Command line arguments analysis */
    if (argc > 1) {

        std::vector<std::string> args;
        for (int i = 1; i < argc; i++) {
            args.push_back(argv[i]);
        }

        /* At least one argument */
        for (const auto& arg : args) {
            std::cout << "arg: " << arg << std::endl;
            if (arg == "-d" || arg == "--default") {
                break;
            }

            if (arg == "-v" || arg == "--verbose") {
                isVerbose = true;
                continue;
            }

            const std::vector<std::string> argument = StringUtils::split(arg, "=");
            if (argument.size() != 2) {
                displayUsageAndExit();
            }

            if (argument[0] == "-a" || argument[0] == "--aid") {
                cardAid = argument[1];
                if (argument[1].length() < 10 ||
                    argument[1].length() > 32 ||
                    !HexUtil::isValid(argument[1])) {
                    std::cout << "Invalid AID";
                    displayUsageAndExit();
                }

            } else if (argument[0] == "-c" || argument[0] == "--card") {
                cardReaderRegex = argument[1];

            } else if (argument[0] == "-s" || argument[0] == "--sam") {
                samReaderRegex = argument[1];

            } else {
                displayUsageAndExit();
            }
        }
    } else {
        displayUsageAndExit();
    }
}


int main(int argc, char **argv)
{
    parseCommandLine(argc, argv);

    Logger::setLoggerLevel(isVerbose ? Logger::Level::logTrace : Logger::Level::logInfo);

    logger->info("=============== UseCase Calypso #10: session trace TN313 ==================\n");

    logger->info("Using parameters:\n");
    logger->info("  AID=%\n", cardAid);
    logger->info("  CARD_READER_REGEX=%\n", cardReaderRegex);
    logger->info("  SAM_READER_REGEX=%\n", samReaderRegex);

    /* Get the instance of the SmartCardService */
    std::shared_ptr<SmartCardService> smartCardService = SmartCardServiceProvider::getService();

    /* Register the PcscPlugin */
    std::shared_ptr<Plugin> plugin =
        smartCardService->registerPlugin(PcscPluginFactoryBuilder::builder()->build());

    /* Get the Calypso card extension service */
    std::shared_ptr<CalypsoExtensionService> calypsoCardService =
        CalypsoExtensionService::getInstance();

     /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(calypsoCardService);

    /* Get the card and SAM readers whose name matches the provided regexs */
    auto cardReader = ConfigurationUtil::getCardReader(plugin, cardReaderRegex);
    auto samReader = ConfigurationUtil::getSamReader(plugin, samReaderRegex);

    /* Get the Calypso SAM SmartCard after selection. */
    std::shared_ptr<CalypsoSam> calypsoSam = ConfigurationUtil::getSam(samReader);

    logger->info("= SAM = %\n", calypsoSam);

    logger->info("Select application with AID = '%'\n", cardAid);

    /* Get the core card selection manager */
    std::shared_ptr<CardSelectionManager> cardSelectionManager =
        smartCardService->createCardSelectionManager();

    /*
     * Create a card selection using the Calypso card extension.
     * Select the card and read the record 1 of the file ENVIRONMENT_AND_HOLDER
     */
    std::shared_ptr<CalypsoCardSelection>  cardSelection = calypsoCardService->createCardSelection();
    cardSelection->acceptInvalidatedCard()
                  .filterByCardProtocol(ConfigurationUtil::ISO_CARD_PROTOCOL)
                  .filterByDfName(cardAid);

    /* Prepare the selection by adding the created Calypso selection to the card selection scenario. */
    cardSelectionManager->prepareSelection(cardSelection);

    /*
     * Schedule the selection scenario, request notification only if the card matches the selection
     * case.
     */
    auto observable = std::dynamic_pointer_cast<ObservableCardReader>(cardReader);
    cardSelectionManager->scheduleCardSelectionScenario(
        observable,
        ObservableCardReader::DetectionMode::REPEATING,
        ObservableCardReader::NotificationMode::MATCHED_ONLY);

    /* Create security settings that reference the SAM */
    std::shared_ptr<CardSecuritySetting> cardSecuritySetting =
        CalypsoExtensionService::getInstance()->createCardSecuritySetting();
    cardSecuritySetting->assignDefaultKif(WriteAccessLevel::PERSONALIZATION, 0x21)
                        .assignDefaultKif(WriteAccessLevel::LOAD, 0x27)
                        .assignDefaultKif(WriteAccessLevel::DEBIT, 0x30)
                        .setControlSamResource(samReader, calypsoSam);

    /* Create and add a card observer for this reader */
    auto cardReaderObserver =
        std::make_shared<CardReaderObserver>(cardReader, cardSelectionManager, cardSecuritySetting);
    observable->setReaderObservationExceptionHandler(cardReaderObserver);
    observable->addObserver(cardReaderObserver);
    observable->startCardDetection(ObservableCardReader::DetectionMode::REPEATING);


    logger->info("Wait for a card...\n");

    while(1);
}
