/**************************************************************************************************
 * Copyright (c) 2021 Calypso Networks Association https://calypsonet.org/                        *
 *                                                                                                *
 * See the NOTICE file(s) distributed with this work for additional information regarding         *
 * copyright ownership.                                                                           *
 *                                                                                                *
 * This program and the accompanying materials are made available under the terms of the Eclipse  *
 * Public License 2.0 which is available at http://www.eclipse.org/legal/epl-2.0                  *
 *                                                                                                *
 * SPDX-License-Identifier: EPL-2.0                                                               *
 **************************************************************************************************/

/* Keyple Card Calypso */
#include "CalypsoExtensionService.h"

/* Keyple Core Service */
#include "ConfigurableReader.h"
#include "SmartCardService.h"
#include "SmartCardServiceProvider.h"

/* Keyple Core Util */
#include "ByteArrayUtil.h"
#include "ContactCardCommonProtocol.h"
#include "ContactlessCardCommonProtocol.h"
#include "IllegalStateException.h"
#include "LoggerFactory.h"
#include "StringUtils.h"

/* Keyple Plugin Pcsc */
#include "PcscPlugin.h"
#include "PcscPluginFactory.h"
#include "PcscPluginFactoryBuilder.h"
#include "PcscReader.h"
#include "PcscSupportedContactlessProtocol.h"

/* Keyple Core Resource */
#include "CardResource.h"
#include "CardResourceServiceProvider.h"

/* Keyple Cpp Example */
#include "CalypsoConstants.h"
#include "CardReaderObserver.h"
#include "ConfigurationUtil.h"

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
 *   <li>Initialize and start the SAM resource service.
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
    std::cout << " -d, --default                  use default values (is equivalent to -a=\"31544" \
              << "9432E49434131\" -c=\".*ASK LoGO.*|.*Contactless.*\" -s=\".*Identive.*|.*HID.*\"" \
              << ")" << std::endl;
    std::cout << " -a, --aid=\"APPLICATION_AID\"    between 5 and 16 hex bytes (e.g. \"315449432E" \
              << "49434131\")" << std::endl;
    std::cout << " -c, --card=\"CARD_READER_REGEX\" regular expression matching the card reader n" \
              << "ame (e.g. \"ASK Logo.*\")" << std::endl;
    std::cout << " -s, --sam=\"SAM_READER_REGEX\"   regular expression matching the SAM reader na" \
              << "me (e.g. \"HID.*\")" << std::endl;
    std::cout << " -v, --verbose                  set the log level to TRACE" << std::endl;

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
                    !ByteArrayUtil::isValidHexString(argument[1])) {
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

    /* Get the instance of the SmartCardService (singleton pattern) */
    std::shared_ptr<SmartCardService> smartCardService = SmartCardServiceProvider::getService();

    /*
     * Register the PcscPlugin with the SmartCardService, get the corresponding generic plugin in
     * return.
     */
    std::shared_ptr<Plugin> plugin =
        smartCardService->registerPlugin(PcscPluginFactoryBuilder::builder()->build());

    /* Get the Calypso card extension service */
    std::shared_ptr<CalypsoExtensionService> cardExtension = CalypsoExtensionService::getInstance();

     /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(cardExtension);

    /* Retrieve the card reader */
    std::shared_ptr<Reader> cardReader = ConfigurationUtil::getCardReader(plugin, cardReaderRegex);

    /* Activate the ISO14443 card protocol */
    std::dynamic_pointer_cast<ConfigurableReader>(cardReader)
        ->activateProtocol(PcscSupportedContactlessProtocol::ISO_14443_4.getName(),
                           ContactlessCardCommonProtocol::ISO_14443_4.getName());

    logger->info("Select application with AID = '%'\n", cardAid);

    /* Get the core card selection manager */
    std::shared_ptr<CardSelectionManager> cardSelectionManager =
        smartCardService->createCardSelectionManager();

    /*
     * Create a card selection using the Calypso card extension.
     * Select the card and read the record 1 of the file ENVIRONMENT_AND_HOLDER
     */
    std::shared_ptr<CalypsoCardSelection>  cardSelection = cardExtension->createCardSelection();
    cardSelection->acceptInvalidatedCard()
                  .filterByCardProtocol(ContactlessCardCommonProtocol::ISO_14443_4.getName())
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

    /* Configure the card resource service for the targeted SAM */
    ConfigurationUtil::setupCardResourceService(plugin, samReaderRegex, CalypsoConstants::SAM_PROFILE_NAME);

    /*
     * Create security settings that reference the same SAM profile requested from the card resource
     * service.
     */
    std::shared_ptr<CardResource> samResource =
        CardResourceServiceProvider::getService()
            ->getCardResource(CalypsoConstants::SAM_PROFILE_NAME);

    std::shared_ptr<CardSecuritySetting> cardSecuritySetting =
        CalypsoExtensionService::getInstance()->createCardSecuritySetting();
    cardSecuritySetting->setSamResource(samResource->getReader(),
                                        std::dynamic_pointer_cast<CalypsoSam>(
                                            samResource->getSmartCard()));

    /* Create and add a card observer for this reader */
    auto cardReaderObserver =
        std::make_shared<CardReaderObserver>(cardReader, cardSelectionManager, cardSecuritySetting);
    observable->setReaderObservationExceptionHandler(cardReaderObserver);
    observable->addObserver(cardReaderObserver);
    observable->startCardDetection(ObservableCardReader::DetectionMode::REPEATING);


    logger->info("Wait for a card...\n");

    while(1);
}
