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
#include <memory>
#include <string>
#include <utility>

#include "keyple/card/calypso/CalypsoExtensionService.hpp"
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
#include "keypop/calypso/card/CalypsoCardApiFactory.hpp"
#include "keypop/calypso/card/card/CalypsoCard.hpp"
#include "keypop/calypso/card/card/CalypsoCardSelectionExtension.hpp"
#include "keypop/calypso/card/transaction/FreeTransactionManager.hpp"
#include "keypop/reader/CardReader.hpp"
#include "keypop/reader/ChannelControl.hpp"
#include "keypop/reader/ReaderApiFactory.hpp"
#include "keypop/reader/selection/BasicCardSelector.hpp"
#include "keypop/reader/selection/CardSelectionManager.hpp"
#include "keypop/reader/selection/CardSelectionResult.hpp"
#include "keypop/reader/selection/spi/SmartCard.hpp"

#include "../common/ConfigurationUtil.hpp"

using keyple::card::calypso::CalypsoExtensionService;
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
using keypop::calypso::card::CalypsoCardApiFactory;
using keypop::calypso::card::card::CalypsoCard;
using keypop::calypso::card::card::CalypsoCardSelectionExtension;
using keypop::calypso::card::transaction::FreeTransactionManager;
using keypop::reader::CardReader;
using keypop::reader::ChannelControl;
using keypop::reader::ReaderApiFactory;
using keypop::reader::selection::BasicCardSelector;
using keypop::reader::selection::CardSelectionManager;
using keypop::reader::selection::CardSelectionResult;
using keypop::reader::selection::spi::SmartCard;

/**
 * Handles the process of explicit selection of a Calypso card Revision 1
 * using the PC/SC plugin, without implementing the observation of the
 * reader. Ensure the Calypso card is inserted before launching the program.
 *
 * <p>This class demonstrates the use of the protocol filtering in the
 * selection phase for card having no AID.
 */
class Main_Rev1Selection_Pcsc { };
static std::unique_ptr<Logger> logger
    = LoggerFactory::getLogger(typeid(Main_Rev1Selection_Pcsc));

/* File identifiers */
static const std::uint8_t SFI_ENVIRONMENT_AND_HOLDER = 0x07;
static const std::uint8_t SFI_EVENT_LOG = 0x08;

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
        PcscSupportedContactlessProtocol::INNOVATRON_B_PRIME_CARD.getName(),
        ConfigurationUtil::INNOVATRON_CARD_PROTOCOL);
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

int
main() {
    logger->info(
        "= UseCase Calypso #3: selection of a rev1 card "
        "==================\n");

    /* Initialize the context */
    initKeypleService();
    initCardReader();
    initCalypsoCardExtensionService();

    /* Check if a card is present in the reader */
    if (!cardReader->isCardPresent()) {
        throw IllegalStateException("No card is present in the reader.");
    }

    logger->info(
        "= #### Select the card by its INNOVATRON protocol (no AID)\n");

    /* Get the core card selection manager */
    std::shared_ptr<CardSelectionManager> cardSelectionManager(
        readerApiFactory->createCardSelectionManager());

    std::shared_ptr<BasicCardSelector> cardSelector(
        readerApiFactory->createBasicCardSelector());
    cardSelector->filterByCardProtocol(
        ConfigurationUtil::INNOVATRON_CARD_PROTOCOL);

    /*
     * Create a card selection using the Calypso card extension.
     * No AID is defined, only the card protocol will be used to define the
     * selection case.
     */
    std::unique_ptr<CalypsoCardSelectionExtension>
        calypsoCardSelectionExtension(
            calypsoCardApiFactory->createCalypsoCardSelectionExtension());
    calypsoCardSelectionExtension->acceptInvalidatedCard().prepareReadRecord(
        SFI_ENVIRONMENT_AND_HOLDER, 1);
    cardSelectionManager->prepareSelection(
        cardSelector, std::move(calypsoCardSelectionExtension));

    /* Actual card communication: run the selection scenario */
    const std::shared_ptr<CardSelectionResult> selectionResult(
        cardSelectionManager->processCardSelectionScenario(cardReader));

    /* Check the selection result */
    if (selectionResult->getActiveSmartCard() == nullptr) {
        throw IllegalStateException(
            "The selection of the B Prime card failed.");
    }

    /* Get the SmartCard resulting of the selection */
    const std::shared_ptr<SmartCard> card(
        selectionResult->getActiveSmartCard());
    auto calypsoCard = std::dynamic_pointer_cast<CalypsoCard>(card);

    logger->info("= SmartCard = %\n", calypsoCard);

    const std::string csn(
        HexUtil::toHex(calypsoCard->getApplicationSerialNumber()));
    logger->info("Calypso Serial Number = %\n", csn);

    /* Performs file reads using the card transaction manager in non-secure mode
     */
    calypsoCardApiFactory->createFreeTransactionManager(cardReader, calypsoCard)
        ->prepareReadRecord(SFI_EVENT_LOG, 1)
        .processCommands(ChannelControl::CLOSE_AFTER);

    const std::string sfiEnvHolder(HexUtil::toHex(SFI_ENVIRONMENT_AND_HOLDER));
    logger->info(
        "File %h, rec 1: FILE_CONTENT = %\n",
        sfiEnvHolder,
        calypsoCard->getFileBySfi(SFI_ENVIRONMENT_AND_HOLDER));

    const std::string sfiEventLog(HexUtil::toHex(SFI_EVENT_LOG));
    logger->info(
        "File %h, rec 1: FILE_CONTENT = %\n",
        sfiEventLog,
        calypsoCard->getFileBySfi(SFI_EVENT_LOG));

    logger->info("= #### End of the Calypso card processing\n");

    return 0;
}
