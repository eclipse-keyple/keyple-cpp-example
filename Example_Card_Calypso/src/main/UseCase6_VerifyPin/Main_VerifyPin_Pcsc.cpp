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
#include "keypop/calypso/card/card/CalypsoCard.hpp"
#include "keypop/calypso/card/card/CalypsoCardSelectionExtension.hpp"
#include "keypop/calypso/card/cpp/SecureRegularModeTransactionManagerBase.hpp"
#include "keypop/calypso/card/transaction/FreeTransactionManager.hpp"
#include "keypop/calypso/card/transaction/InvalidPinException.hpp"
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
using keyple::plugin::pcsc::PcscPluginFactoryBuilder;
using keyple::plugin::pcsc::PcscReader;
using keyple::plugin::pcsc::PcscSupportedContactlessProtocol;
using keyple::plugin::pcsc::PcscSupportedContactProtocol;
using keypop::calypso::card::CalypsoCardApiFactory;
using keypop::calypso::card::WriteAccessLevel;
using keypop::calypso::card::card::CalypsoCard;
using keypop::calypso::card::card::CalypsoCardSelectionExtension;
using keypop::calypso::card::cpp::SecureRegularModeTransactionManagerBase;
using keypop::calypso::card::transaction::FreeTransactionManager;
using keypop::calypso::card::transaction::InvalidPinException;
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
 * Manages the process of verifying the PIN code of a Calypso card using the
 * PC/SC plugin, demonstrating both plain and encrypted PIN verification
 * methods.
 */
class Main_VerifyPin_Pcsc { };
static std::unique_ptr<Logger> logger
    = LoggerFactory::getLogger(typeid(Main_VerifyPin_Pcsc));

/** AID: Keyple test kit profile 1, Application 2 */
static const std::string AID = "A000000291FF9101";

static const std::vector<std::uint8_t> PIN_OK = {0x30, 0x30, 0x30, 0x30};
static const std::vector<std::uint8_t> PIN_KO = {0x30, 0x30, 0x30, 0x31};
static const std::uint8_t PIN_VERIFICATION_CIPHERING_KEY_KIF = 0x30;
static const std::uint8_t PIN_VERIFICATION_CIPHERING_KEY_KVC = 0x74;

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
        ConfigurationUtil::CARD_READER_NAME_REGEX,
        true,
        PcscReader::IsoProtocol::T1,
        PcscReader::SharingMode::SHARED,
        PcscSupportedContactlessProtocol::ISO_14443_4.getName(),
        ConfigurationUtil::ISO_CARD_PROTOCOL);
}

/**
 * Initializes the SAM reader with specific configurations.
 */
static void
initSamReader() {
    samReader = ConfigurationUtil::getReader(
        plugin,
        ConfigurationUtil::SAM_READER_NAME_REGEX,
        false,
        PcscReader::IsoProtocol::ANY,
        PcscReader::SharingMode::SHARED,
        PcscSupportedContactProtocol::ISO_7816_3_T0.getName(),
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
        throw IllegalStateException(
            "The selection of the application '" + aid + "' failed.");
    }

    const std::shared_ptr<SmartCard> card(
        selectionResult->getActiveSmartCard());

    return std::dynamic_pointer_cast<CalypsoCard>(card);
}

static int
runExample() {
    logger->info(
        "= UseCase Calypso #6: Calypso card Verify PIN ==================\n");

    /* Initialize the context */
    initKeypleService();
    initCalypsoCardExtensionService();
    initCardReader();
    initSamReader();
    initSecuritySetting();

    /* Verify if a card is present in the reader */
    if (!cardReader->isCardPresent()) {
        throw IllegalStateException("No card is present in the reader.");
    }

    /* Select the card */
    std::shared_ptr<CalypsoCard> calypsoCard = selectCard(cardReader, AID);

    logger->info("= SmartCard = %\n", calypsoCard);

    const std::string csn(
        HexUtil::toHex(calypsoCard->getApplicationSerialNumber()));
    logger->info("Calypso Serial Number = %\n", csn);

    /* Instantiate a Free Transaction manager to operate PIN verification
     * without encryption */
    std::unique_ptr<FreeTransactionManager> freeTransactionManager(
        calypsoCardApiFactory->createFreeTransactionManager(
            cardReader, calypsoCard));

    /* Verify the PIN in plain mode without initiating a secure session */
    freeTransactionManager->prepareVerifyPin(PIN_OK).processCommands(
        ChannelControl::KEEP_OPEN);
    logger->info(
        "Remaining attempts #1: %\n", calypsoCard->getPinAttemptRemaining());

    /* Add the key identifiers needed for ciphering the PIN */
    symmetricCryptoSecuritySetting->setPinVerificationCipheringKey(
        PIN_VERIFICATION_CIPHERING_KEY_KIF, PIN_VERIFICATION_CIPHERING_KEY_KVC);

    /*
     * Instantiate a Secure Regular Mode Transaction Manager to handle
     * encrypted PIN verification and secure operations.
     *
     * The keypop API declares createSecureRegularModeTransactionManager() as
     * returning a SecureRegularModeTransactionManagerBase, which does not
     * itself expose prepareOpenSecureSession() (it's only declared on
     * SecureSymmetricCryptoTransactionManager<T>, a sibling interface
     * implemented by the same concrete object). A downcast is required to
     * reach it.
     */
    std::unique_ptr<SecureRegularModeTransactionManagerBase>
        secureRegularModeTransactionManagerBase(
            calypsoCardApiFactory->createSecureRegularModeTransactionManager(
                cardReader, calypsoCard, symmetricCryptoSecuritySetting));

    auto secureRegularModeTransactionManager
        = dynamic_cast<SecureSymmetricCryptoTransactionManager<
            SecureRegularModeTransactionManagerBase>*>(
            secureRegularModeTransactionManagerBase.get());

    /* Verify the PIN in encrypted mode, outside a secure session */
    secureRegularModeTransactionManager->prepareVerifyPin(PIN_OK)
        .processCommands(ChannelControl::KEEP_OPEN);

    /* Log the current counter value (should be 3) */
    logger->info(
        "Remaining attempts #2: %\n", calypsoCard->getPinAttemptRemaining());

    /*
     * Attempt PIN verification with an incorrect PIN within a secure session,
     * handle exceptions and cancel the session if necessary.
     */
    secureRegularModeTransactionManager->prepareOpenSecureSession(
        WriteAccessLevel::DEBIT);
    try {
        secureRegularModeTransactionManager->prepareVerifyPin(PIN_KO)
            .processCommands(ChannelControl::KEEP_OPEN);
    } catch (const InvalidPinException& ex) {
        logger->error("PIN Exception: %\n", ex.what());
        secureRegularModeTransactionManager->prepareCancelSecureSession()
            .processCommands(ChannelControl::KEEP_OPEN);
    }

    /* Log the current counter value (should be 2) */
    logger->error(
        "Remaining attempts #3: %\n", calypsoCard->getPinAttemptRemaining());

    /* Initiate a secure session, verify the PIN correctly, and then close the
     * session */
    secureRegularModeTransactionManager
        ->prepareOpenSecureSession(WriteAccessLevel::DEBIT)
        .prepareCheckPinStatus()
        .processCommands(ChannelControl::KEEP_OPEN);

    /* Log the current counter value (should be 2) */
    logger->info(
        "Remaining attempts #4: %\n", calypsoCard->getPinAttemptRemaining());

    secureRegularModeTransactionManager->prepareVerifyPin(PIN_OK)
        .prepareCloseSecureSession()
        .processCommands(ChannelControl::CLOSE_AFTER);

    /* Log the current counter value (should be 3) */
    logger->info(
        "Remaining attempts #5: %\n", calypsoCard->getPinAttemptRemaining());

    logger->info(
        "The Secure Session ended successfully, the PIN has been "
        "verified.\n");

    logger->info("= #### End of the Calypso card processing\n");

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
