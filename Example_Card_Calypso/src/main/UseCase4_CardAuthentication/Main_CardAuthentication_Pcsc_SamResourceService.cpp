/**************************************************************************************************
 * Copyright (c) 2022 Calypso Networks Association https://calypsonet.org/                        *
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
#include "ConfigurableCardReader.h"

/* Keyple Card Calypso */
#include "CalypsoExtensionService.h"

/* Keyple Core Service */
#include "ConfigurableReader.h"
#include "SmartCardService.h"
#include "SmartCardServiceProvider.h"

/* Keyple Core Util */
#include "ContactCardCommonProtocol.h"
#include "ContactlessCardCommonProtocol.h"
#include "HexUtil.h"
#include "IllegalStateException.h"
#include "LoggerFactory.h"
#include "StringUtils.h"

/* Keyple Plugin Pcsc */
#include "PcscPlugin.h"
#include "PcscPluginFactory.h"
#include "PcscPluginFactoryBuilder.h"
#include "PcscReader.h"
#include "PcscSupportedContactlessProtocol.h"
#include "PcscSupportedContactProtocol.h"

/* Keyple Core Resource */
#include "CardResource.h"
#include "CardResourceServiceProvider.h"

/* Keyple Cpp Example */
#include "CalypsoConstants.h"
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
 *
 *
 * <h1>Use Case Calypso 4 – Calypso Card authentication (PC/SC)</h1>
 *
 * <p>We demonstrate here the authentication of a Calypso card using a Secure Session in which a
 * file from the card is read. The read is certified by verifying the signature of the card by a
 * Calypso SAM.
 *
 * <p>Two readers are required for this example: a contactless reader for the Calypso Card, a
 * contact reader for the Calypso SAM.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Sets up the card resource service to provide a Calypso SAM (C1).
 *   <li>Checks if an ISO 14443-4 card is in the reader, enables the card selection manager.
 *   <li>Attempts to select the specified card (here a Calypso card characterized by its AID) with
 *       an AID-based application selection scenario.
 *   <li>Creates a {@link CardTransactionManager} using {@link CardSecuritySetting} referencing the
 *       SAM profile defined in the card resource service.
 *   <li>Read a file record in Secure Session.
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_CardAuthentication_Pcsc_SamResourceService {};
static const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_CardAuthentication_Pcsc_SamResourceService));

int main()
{
    /* Get the instance of the SmartCardService (singleton pattern) */
    std::shared_ptr<SmartCardService> smartCardService = SmartCardServiceProvider::getService();

    /*
     * Register the PcscPlugin with the SmartCardService, get the corresponding generic plugin in
     * return.
     */
    std::shared_ptr<Plugin> plugin =
        smartCardService->registerPlugin(PcscPluginFactoryBuilder::builder()->build());

    /* Get the Calypso card extension service */
    std::shared_ptr<CalypsoExtensionService> calypsoCardService =
        CalypsoExtensionService::getInstance();

    /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(calypsoCardService);

    /* Get the contactless reader whose name matches the provided regex */
    const std::string pcscContactlessCardReaderName =
        ConfigurationUtil::getCardReaderName(plugin, ConfigurationUtil::CARD_READER_NAME_REGEX);
    std::shared_ptr<CardReader> cardReader = plugin->getReader(pcscContactlessCardReaderName);

    /* Configure the reader with parameters suitable for contactless operations. */
    std::dynamic_pointer_cast<PcscReader>(
        plugin->getReaderExtension(typeid(PcscReader), pcscContactlessCardReaderName))
            ->setContactless(true)
             .setIsoProtocol(PcscReader::IsoProtocol::T1)
             .setSharingMode(PcscReader::SharingMode::SHARED);

    std::dynamic_pointer_cast<ConfigurableCardReader>(cardReader)
        ->activateProtocol(PcscSupportedContactlessProtocol::ISO_14443_4.getName(),
                           ConfigurationUtil::ISO_CARD_PROTOCOL);

    /*
     * Configure the card resource service to provide an adequate SAM for future secure operations.
     * We suppose here, we use an Identive contact PC/SC reader as card reader.
     */
    ConfigurationUtil::setupCardResourceService(plugin,
                                                ConfigurationUtil::SAM_READER_NAME_REGEX,
                                                CalypsoConstants::SAM_PROFILE_NAME);

    logger->info("=============== " \
                 "UseCase Calypso #4: Calypso card authentication " \
                 "==================\n");

    /* Check if a card is present in the reader */
    if (!cardReader->isCardPresent()) {
        throw IllegalStateException("No card is present in the reader.");
    }

    logger->info("= #### Select application with AID = '%'\n", CalypsoConstants::AID);

    /* Get the core card selection manager */
    std::shared_ptr<CardSelectionManager> cardSelectionManager =
        smartCardService->createCardSelectionManager();

    /*
     * Create a card selection using the Calypso card extension.
     * Prepare the selection by adding the created Calypso card selection to the card selection
     * scenario.
     */
    auto cardSelection = calypsoCardService->createCardSelection();
    cardSelection->acceptInvalidatedCard().filterByDfName(CalypsoConstants::AID);
    cardSelectionManager->prepareSelection(cardSelection);

    /* Actual card communication: run the selection scenario. */
    std::shared_ptr<CardSelectionResult> selectionResult =
        cardSelectionManager->processCardSelectionScenario(cardReader);

    /* Check the selection result. */
    if (selectionResult->getActiveSmartCard() == nullptr) {
        throw IllegalStateException("The selection of the application " +
                                    CalypsoConstants::AID +
                                    " failed.");
    }

    /* Get the SmartCard resulting of the selection. */
    std::shared_ptr<CalypsoCard> calypsoCard =
        std::dynamic_pointer_cast<CalypsoCard>(selectionResult->getActiveSmartCard());

    logger->info("= SmartCard = %\n", calypsoCard);

    const std::string csn = HexUtil::toHex(calypsoCard->getApplicationSerialNumber());
    logger->info("Calypso Serial Number = %\n", csn);

    /*
     * Create security settings that reference the same SAM profile requested from the card resource
     * service.
     */
    std::shared_ptr<CardResource> samResource =
        CardResourceServiceProvider::getService()
            ->getCardResource(CalypsoConstants::SAM_PROFILE_NAME);
    std::shared_ptr<CardSecuritySetting> cardSecuritySetting =
        CalypsoExtensionService::getInstance()->createCardSecuritySetting();
    cardSecuritySetting->setControlSamResource(
        samResource->getReader(),
        std::dynamic_pointer_cast<CalypsoSam>(samResource->getSmartCard()));

    try {
        /* Performs file reads using the card transaction manager in secure mode. */
        calypsoCardService->createCardTransaction(cardReader, calypsoCard, cardSecuritySetting)
                          ->prepareReadRecords(CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER,
                                               CalypsoConstants::RECORD_NUMBER_1,
                                               CalypsoConstants::RECORD_NUMBER_1,
                                               CalypsoConstants::RECORD_SIZE)
                           .processOpening(WriteAccessLevel::DEBIT)
                           .prepareReleaseCardChannel()
                           .processClosing();

    } catch (const Exception& e) {
        (void)e;
    }

    /* Finally */
    try {
        CardResourceServiceProvider::getService()->releaseCardResource(samResource);
    } catch (const RuntimeException& e) {
        logger->error("Error during the card resource release: %\n", e.getMessage(), e);
    }

    logger->info("The Secure Session ended successfully, the card is authenticated and the data " \
                 "read are certified\n");

    const std::string sfiEnvHolder = HexUtil::toHex(CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER);
    logger->info("File %h, rec 1: FILE_CONTENT = %\n",
                 sfiEnvHolder,
                 calypsoCard->getFileBySfi(CalypsoConstants::SFI_ENVIRONMENT_AND_HOLDER));

    logger->info("= #### End of the Calypso card processing.\n");

    return 0;
}
