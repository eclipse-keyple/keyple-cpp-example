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

/* Keyple Core Util */
#include "HexUtil.h"
#include "LoggerFactory.h"
#include "System.h"
#include "Thread.h"

/* Keyple Core Service */
#include "ConfigurableReader.h"
#include "SmartCardService.h"
#include "SmartCardServiceProvider.h"

/* Keyple Plugin Pcsc */
#include "PcscPlugin.h"
#include "PcscPluginFactory.h"
#include "PcscPluginFactoryBuilder.h"

/* Keyple Service Resource */
#include "CardResourceProfileConfigurator.h"
#include "CardResourceService.h"
#include "CardResourceServiceProvider.h"
#include "PluginsConfigurator.h"

/* Keyple Core Common */
#include "KeyplePluginExtension.h"
#include "KeypleReaderExtension.h"

/* Keyple Cpp Example */
#include "CalypsoConstants.h"
#include "ConfigurationUtil.h"

using namespace calypsonet::terminal::reader;
using namespace keyple::card::calypso;
using namespace keyple::core::common;
using namespace keyple::core::service;
using namespace keyple::core::service::resource;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp;
using namespace keyple::plugin::pcsc;

/**
 * Use Case Calypso 12 � Performance measurement: reloading (PC/SC)
 *
 * <p>This code is dedicated to performance measurement for a reloading type transaction.
 *
 * <p>It implements the scenario described <a
 * href="https://terminal-api.calypsonet.org/apis/calypsonet-terminal-calypso-api/#simple-secure-\
 * session-for-an-efficient-distributed-system">here</a>:
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 */
class Main_PerformanceMeasurement_DistributedReloading_Pcsc {};
const std::unique_ptr<Logger> logger =
LoggerFactory::getLogger(typeid(Main_PerformanceMeasurement_DistributedReloading_Pcsc));

/* User interface management */
static const std::string RESET = "\u001B[0m";
static const std::string RED = "\u001B[31m";
static const std::string GREEN = "\u001B[32m";
static const std::string YELLOW = "\u001B[33m";

/* Operating parameters */
static const std::string cardReaderRegex =
    ".*ASK LoGO.*|.*Contactless.*|.*ACR122U.*|.*00 01.*|.*5x21-CL 0.*";
static const std::string samReaderRegex =
    ".*Identive.*|.*HID.*|.*SAM.*|.*00 00.*|.*5x21 0.*";
static const std::string cardAid = "315449432E49434131";
static const int counterIncrement = 10;
static const std::string logLevel = "INFO";
static const std::vector<uint8_t> newContractListRecord =
    HexUtil::toByteArray("00112233445566778899AABBCCDDEEFF00112233445566778899AABBCC");
static const std::vector<uint8_t> newContractRecord =
HexUtil::toByteArray("AABBCCDDEEFFAABBCCDDEEFFAABBCCDDEEFFAABBCCDDEEFFAABBCCDDEE");
static std::string builtDate = __DATE__;
static std::string builtTime = __TIME__;

int main()
{
    logger->info("%=============== Performance measurement: validation transaction "\
                 "===============\n", GREEN);
    logger->info("Using parameters:\n");
    logger->info("  CARD_READER_REGEX=%\n", cardReaderRegex);
    logger->info("  SAM_READER_REGEX=%\n", samReaderRegex);
    logger->info("  AID=%\n", cardAid);
    logger->info("  Counter increment=%\n", counterIncrement);
    logger->info("  log level=%\n", logLevel);
    logger->info("Build data: %s %s%s\n", builtDate, builtTime, RESET);

    /* Get the main Keyple service */
    std::shared_ptr<SmartCardService> smartCardService = SmartCardServiceProvider::getService();

    /* Register the PcscPlugin */
    std::shared_ptr<Plugin> plugin =
        smartCardService->registerPlugin(PcscPluginFactoryBuilder::builder()->build());

    /* Get the card readers whose name matches the provided regex */
    std::shared_ptr<CardReader> cardReader =
        ConfigurationUtil::getCardReader(plugin, ConfigurationUtil::CARD_READER_NAME_REGEX);

    /* Get the Calypso card extension service */
    std::shared_ptr<CalypsoExtensionService> calypsoCardService =
        CalypsoExtensionService::getInstance();

    /* Verify that the extension's API level is consistent with the current service. */
    smartCardService->checkCardExtension(calypsoCardService);

    /* Get the core card selection manager. */
    std::shared_ptr<CardSelectionManager> cardSelectionManager =
        smartCardService->createCardSelectionManager();

    /*
     * Create a card selection using the Calypso card extension.
     * Select the card and read the record 1 of the file ENVIRONMENT_AND_HOLDER
     * Prepare the selection by adding the selection to the card selection
     * scenario.
     */
    std::shared_ptr<CalypsoCardSelection> selection = calypsoCardService->createCardSelection();
    selection->acceptInvalidatedCard()
        .filterByCardProtocol(ConfigurationUtil::ISO_CARD_PROTOCOL)
        .filterByDfName(cardAid)
        .prepareReadRecord(CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER,
                           CalypsoConstants::RECORD_NUMBER_1)
        .prepareReadRecord(CalypsoConstants::SFI_CONTRACT_LIST, CalypsoConstants::RECORD_NUMBER_1);
    cardSelectionManager->prepareSelection(selection);

    /* Configure the card resource service for the targeted SAM. */
    ConfigurationUtil::setupCardResourceService(plugin,
                                                samReaderRegex, CalypsoConstants::SAM_PROFILE_NAME);

    /*
     * Create security settings that reference the same SAM profile requested from the card resource
     * service.
     */
    std::shared_ptr<CardResource> samResource =
        CardResourceServiceProvider::getService()
            ->getCardResource(CalypsoConstants::SAM_PROFILE_NAME);

    if (samResource == nullptr) {
        throw IllegalStateException("No SAM resource available.");
    }

    logger->info("Calypso SAM = %\n", samResource->getSmartCard());

    std::shared_ptr<CardSecuritySetting> cardSecuritySetting =
        CalypsoExtensionService::getInstance()->createCardSecuritySetting();
    cardSecuritySetting->setControlSamResource(
        samResource->getReader(),
        std::dynamic_pointer_cast<CalypsoSam>(samResource->getSmartCard()));

    while (true) {
        logger->info("%########################################################%\n", YELLOW, RESET);
        logger->info("%## Press ENTER when the card is in the reader's field ##%\n", YELLOW, RESET);
        logger->info("%## (or press 'q' + ENTER to exit)                     ##%\n", YELLOW, RESET);
        logger->info("%########################################################%\n", YELLOW, RESET);

        char key = static_cast<char>(getchar());

        if (key == 'q') {
            break;
        }

        if (cardReader->isCardPresent()) {
            try {
                logger->info("Starting reloading transaction...\n");
                logger->info("Select application with AID = '%'\n", cardAid);

                /* Read the current time used later to compute the transaction time */
                long timeStamp = System::currentTimeMillis();

                /* Process the card selection scenario */
                std::shared_ptr<CardSelectionResult> cardSelectionResult =
                    cardSelectionManager->processCardSelectionScenario(cardReader);
                auto calypsoCard =
                    std::dynamic_pointer_cast<CalypsoCard>(
                        cardSelectionResult->getActiveSmartCard());
                if (calypsoCard == nullptr) {
                    throw IllegalStateException("Card selection failed!");
                }

                std::vector<uint8_t> environmentAndHolderData =
                    calypsoCard->getFileBySfi(CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER)
                               ->getData()
                               ->getContent(CalypsoConstants::RECORD_NUMBER_1);

                std::vector<uint8_t> contractListData =
                    calypsoCard->getFileBySfi(CalypsoConstants::SFI_CONTRACT_LIST)
                               ->getData()
                               ->getContent(CalypsoConstants::RECORD_NUMBER_1);

                /* TODO Place here the analysis of the contextand the last event log */

                /*
                 * Create a transaction manager, open a Secure Session, read Environmentand Event
                 * Log.
                 */
                std::shared_ptr<CardTransactionManager> cardTransactionManager =
                    CalypsoExtensionService::getInstance()
                    ->createCardTransaction(cardReader, calypsoCard, cardSecuritySetting);
                cardTransactionManager->prepareReadRecord(
                    CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER, CalypsoConstants::RECORD_NUMBER_1)
                                       .prepareReadRecord(
                                           CalypsoConstants::SFI_CONTRACT_LIST,
                                           CalypsoConstants::RECORD_NUMBER_1)
                                       .prepareReadRecords(
                                           CalypsoConstants::SFI_CONTRACTS,
                                           CalypsoConstants::RECORD_NUMBER_1,
                                           CalypsoConstants::RECORD_NUMBER_2,
                                           CalypsoConstants::RECORD_SIZE)
                                       .prepareReadCounter(CalypsoConstants::SFI_COUNTERS, 2)
                                       .processOpening(WriteAccessLevel::LOAD);

                environmentAndHolderData =
                    calypsoCard->getFileBySfi(CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER)
                               ->getData()
                               ->getContent(CalypsoConstants::RECORD_NUMBER_1);

                contractListData = calypsoCard->getFileBySfi(CalypsoConstants::SFI_CONTRACT_LIST)
                                              ->getData()
                                              ->getContent(CalypsoConstants::RECORD_NUMBER_1);

                const std::vector<uint8_t> contract1Data =
                    calypsoCard->getFileBySfi(CalypsoConstants::SFI_CONTRACTS)
                               ->getData()
                               ->getContent(CalypsoConstants::RECORD_NUMBER_1);

                const std::vector<uint8_t> contract2Data =
                    calypsoCard->getFileBySfi(CalypsoConstants::SFI_CONTRACTS)
                               ->getData()
                               ->getContent(CalypsoConstants::RECORD_NUMBER_2);

                // const int counter1Value =
                //     *(calypsoCard->getFileBySfi(CalypsoConstants::SFI_CONTRACT_LIST)
                //                  ->getData()
                //                  ->getContentAsCounterValue(1));

                // const int counter2Value =
                //     *(calypsoCard->getFileBySfi(CalypsoConstants::SFI_CONTRACT_LIST)
                //                  ->getData()
                //                  ->getContentAsCounterValue(2));

                /*
                 * TODO Place here the analysis of the context, the contract list, the contracts,
                 * the counters and the preparation of the card's content update
                 */

                /* Update contract listand contract, increase counterand close the Secure Session */
                cardTransactionManager->prepareUpdateRecord(CalypsoConstants::SFI_CONTRACT_LIST,
                                                            CalypsoConstants::RECORD_NUMBER_1,
                                                            newContractListRecord)
                                       .prepareUpdateRecord(CalypsoConstants::SFI_CONTRACTS,
                                                            CalypsoConstants::RECORD_NUMBER_1,
                                                            newContractRecord)
                                       .prepareIncreaseCounter(CalypsoConstants::SFI_COUNTERS,
                                                               1,
                                                               counterIncrement)
                                       .prepareReleaseCardChannel()
                                       .processClosing();

                /* Display transaction time */
                logger->info("%Transaction succeeded. Execution time: % ms%\n",
                             GREEN,
                             System::currentTimeMillis() - timeStamp,
                             RESET);

            } catch (const Exception& e) {
                logger->error("%Transaction failed with exception: %%\n",
                              RED,
                              e.getMessage(),
                              RESET);
            }
        }
    }

    logger->info("Exiting the program on user's request.\n");
}
