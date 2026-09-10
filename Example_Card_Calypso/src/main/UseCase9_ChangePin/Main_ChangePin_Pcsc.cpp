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
#include <regex>
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
#include "keyple/core/util/cpp/Thread.hpp"
#include "keyple/core/util/cpp/exception/IllegalStateException.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keyple/plugin/pcsc/PcscSupportedContactProtocol.hpp"
#include "keyple/plugin/pcsc/PcscSupportedContactlessProtocol.hpp"
#include "keypop/calypso/card/CalypsoCardApiFactory.hpp"
#include "keypop/calypso/card/card/CalypsoCard.hpp"
#include "keypop/calypso/card/card/CalypsoCardSelectionExtension.hpp"
#include "keypop/calypso/card/cpp/SecureRegularModeTransactionManagerBase.hpp"
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
using keyple::core::util::cpp::Thread;
using keyple::core::util::cpp::exception::IllegalStateException;
using keyple::plugin::pcsc::PcscPluginFactoryBuilder;
using keyple::plugin::pcsc::PcscReader;
using keyple::plugin::pcsc::PcscSupportedContactlessProtocol;
using keyple::plugin::pcsc::PcscSupportedContactProtocol;
using keypop::calypso::card::CalypsoCardApiFactory;
using keypop::calypso::card::card::CalypsoCard;
using keypop::calypso::card::card::CalypsoCardSelectionExtension;
using keypop::calypso::card::cpp::SecureRegularModeTransactionManagerBase;
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
 * Manages the process of changing the PIN code of a Calypso card using the
 * PC/SC plugin, demonstrating both plain and encrypted PIN verification
 * methods.
 *
 * <strong>Warning:</strong> On recent Windows versions, delayed PIN entry can
 * cause loss of communication with the smart card (the card is reset after 5
 * seconds of inactivity).
 */
class Main_ChangePin_Pcsc { };
static std::unique_ptr<Logger> logger
    = LoggerFactory::getLogger(typeid(Main_ChangePin_Pcsc));

/** AID: Keyple test kit profile 1, Application 2 */
static const std::string AID = "315449432E49434131";

static const std::uint8_t PIN_MODIFICATION_CIPHERING_KEY_KIF = 0x21;
static const std::uint8_t PIN_MODIFICATION_CIPHERING_KEY_KVC = 0x79;
static const std::uint8_t PIN_VERIFICATION_CIPHERING_KEY_KIF = 0x30;
static const std::uint8_t PIN_VERIFICATION_CIPHERING_KEY_KVC = 0x79;

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

int
main() {
    logger->info(
        "= UseCase Calypso #9: Calypso card Change PIN ==================\n");

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

    /* Add the key identifiers needed for ciphering the PIN */
    symmetricCryptoSecuritySetting
        ->setPinVerificationCipheringKey(
            PIN_VERIFICATION_CIPHERING_KEY_KIF,
            PIN_VERIFICATION_CIPHERING_KEY_KVC)
        .setPinModificationCipheringKey(
            PIN_MODIFICATION_CIPHERING_KEY_KIF,
            PIN_MODIFICATION_CIPHERING_KEY_KVC);

    /*
     * Instantiate a Secure Regular Mode Transaction Manager to handle
     * encrypted PIN verification and secure operations.
     */
    std::unique_ptr<SecureRegularModeTransactionManagerBase> cardTransaction(
        calypsoCardApiFactory->createSecureRegularModeTransactionManager(
            cardReader, calypsoCard, symmetricCryptoSecuritySetting));

    /* Short delay to allow logs to be displayed before the prompt */
    Thread::sleep(2000);

    std::string inputString;
    std::vector<std::uint8_t> newPinCode;
    bool validPinCodeEntered = false;
    do {
        std::cout << "Enter new PIN code (4 numeric digits):";
        std::getline(std::cin, inputString);
        if (!std::regex_match(inputString, std::regex("[0-9]{4}"))) {
            std::cout << "Invalid PIN code." << std::endl;
        } else {
            newPinCode.assign(inputString.begin(), inputString.end());
            validPinCodeEntered = true;
        }
    } while (!validPinCodeEntered);

    /* Change the PIN (correct) */
    cardTransaction->prepareChangePin(newPinCode)
        .processCommands(ChannelControl::KEEP_OPEN);

    logger->info("PIN code value successfully updated to %\n", inputString);

    /* Verification of the PIN */
    cardTransaction->prepareVerifyPin(newPinCode)
        .processCommands(ChannelControl::CLOSE_AFTER);
    logger->info(
        "Remaining attempts: %\n", calypsoCard->getPinAttemptRemaining());

    logger->info("PIN % code successfully presented.\n", inputString);

    logger->info("= #### End of the Calypso card processing\n");

    return 0;
}
