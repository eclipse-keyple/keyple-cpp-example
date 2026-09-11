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
#include "keyple/core/service/resource/CardResource.hpp"
#include "keyple/core/service/resource/CardResourceProfileConfigurator.hpp"
#include "keyple/core/service/resource/CardResourceService.hpp"
#include "keyple/core/service/resource/CardResourceServiceProvider.hpp"
#include "keyple/core/service/resource/PluginsConfigurator.hpp"
#include "keyple/core/service/resource/spi/CardResourceProfileExtension.hpp"
#include "keyple/core/service/resource/spi/ReaderConfiguratorSpi.hpp"
#include "keyple/core/util/HexUtil.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keyple/core/util/cpp/exception/Exception.hpp"
#include "keyple/core/util/cpp/exception/IllegalStateException.hpp"
#include "keyple/plugin/pcsc/PcscCardCommunicationProtocol.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keypop/calypso/card/CalypsoCardApiFactory.hpp"
#include "keypop/calypso/card/WriteAccessLevel.hpp"
#include "keypop/calypso/card/card/CalypsoCard.hpp"
#include "keypop/calypso/card/card/CalypsoCardSelectionExtension.hpp"
#include "keypop/calypso/card/cpp/SecureRegularModeTransactionManagerBase.hpp"
#include "keypop/calypso/card/transaction/SecureSymmetricCryptoTransactionManager.hpp"
#include "keypop/calypso/card/transaction/SymmetricCryptoSecuritySetting.hpp"
#include "keypop/calypso/crypto/legacysam/LegacySamApiFactory.hpp"
#include "keypop/calypso/crypto/legacysam/sam/LegacySam.hpp"
#include "keypop/calypso/crypto/legacysam/sam/LegacySamSelectionExtension.hpp"
#include "keypop/calypso/crypto/legacysam/spi/LegacySamDynamicUnlockDataProviderSpi.hpp"
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
using keyple::core::service::resource::CardResource;
using keyple::core::service::resource::CardResourceProfileConfigurator;
using keyple::core::service::resource::CardResourceService;
using keyple::core::service::resource::CardResourceServiceProvider;
using keyple::core::service::resource::PluginsConfigurator;
using keyple::core::service::resource::spi::CardResourceProfileExtension;
using keyple::core::service::resource::spi::ReaderConfiguratorSpi;
using keyple::core::util::HexUtil;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keyple::core::util::cpp::exception::Exception;
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
using keypop::calypso::crypto::legacysam::sam::LegacySamSelectionExtension;
using keypop::calypso::crypto::legacysam::spi::
    LegacySamDynamicUnlockDataProviderSpi;
using keypop::reader::CardReader;
using keypop::reader::ChannelControl;
using keypop::reader::ReaderApiFactory;
using keypop::reader::selection::CardSelectionManager;
using keypop::reader::selection::CardSelectionResult;
using keypop::reader::selection::IsoCardSelector;
using keypop::reader::selection::spi::SmartCard;

/**
 * Handles the process of a Calypso card authentication using the PC/SC
 * plugin, the Calypso Card Extension Service, and the Card Resource Service
 * to manage the SAM.
 */
class Main_CardAuthentication_Pcsc_SamResourceService { };
static std::unique_ptr<Logger> logger = LoggerFactory::getLogger(
    typeid(Main_CardAuthentication_Pcsc_SamResourceService));

/** AID: Keyple test kit profile 1, Application 2 */
static const std::string AID = "A000000291FF9101";

/* File identifiers */
static const std::uint8_t SFI_ENVIRONMENT_AND_HOLDER = 0x07;
static const int RECORD_SIZE = 29;

/*
 * The name of the SAM resource provided by the Card Resource Manager and used
 * during the card transaction.
 */
static const std::string SAM_PROFILE_NAME = "SAM C1";

/* The plugin used to manage the readers. */
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
/* The security settings for the card transaction. */
static std::shared_ptr<SymmetricCryptoSecuritySetting>
    symmetricCryptoSecuritySetting;

/**
 * Provides the dynamic unlock data expected by the SAM.
 */
class DynamicUnlockDataProvider final
: public LegacySamDynamicUnlockDataProviderSpi {
public:
    /**
     * {@inheritDoc}
     */
    const std::vector<std::uint8_t>&
    getUnlockData(
        const std::vector<std::uint8_t>& samSerialNumber,
        const std::vector<std::uint8_t>& samChallenge) const override {
        mLogger->debug(
            "DynamicUnlockDataProvider.getUnlockData: samSerialNumber = "
            "%\n",
            HexUtil::toHex(samSerialNumber));
        mLogger->debug(
            "DynamicUnlockDataProvider.getUnlockData: samChallenge = %\n",
            HexUtil::toHex(samChallenge));

        return mUnlockData;
    }

private:
    /**
     *
     */
    const std::unique_ptr<Logger> mLogger
        = LoggerFactory::getLogger(typeid(DynamicUnlockDataProvider));

    /**
     *
     */
    const std::vector<std::uint8_t> mUnlockData
        = HexUtil::toByteArray("0011223344556677");
};

/**
 * Reader configurator used by the card resource service to set up the SAM
 * reader with the required settings.
 */
class ReaderConfigurator final : public ReaderConfiguratorSpi {
public:
    /**
     * {@inheritDoc}
     */
    void
    setupReader(std::shared_ptr<CardReader> reader) override {
        /* Configure the reader with parameters suitable for contact operations
         */
        try {
            auto readerExtension = std::dynamic_pointer_cast<PcscReader>(
                SmartCardServiceProvider::getService()
                    ->getPlugin(reader)
                    ->getReaderExtension(
                        typeid(PcscReader), reader->getName()));
            if (readerExtension != nullptr) {
                readerExtension->setContactless(false)
                    .setIsoProtocol(PcscReader::IsoProtocol::ANY)
                    .setSharingMode(PcscReader::SharingMode::SHARED);
            }
        } catch (const Exception& e) {
            mLogger->error(
                "Exception raised while setting up the reader %\n",
                reader->getName(),
                e);
        }
    }

private:
    /**
     *
     */
    const std::unique_ptr<Logger> mLogger
        = LoggerFactory::getLogger(typeid(ReaderConfigurator));
};

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
        PcscCardCommunicationProtocol::ISO_14443_4.getName(),
        ConfigurationUtil::ISO_CARD_PROTOCOL);
}

/**
 * Initializes the SAM Resource Service making a SAM resource available under
 * the SAM_PROFILE_NAME name.
 */
static void
initSamResourceService() {
    std::shared_ptr<LegacySamApiFactory> legacySamApiFactory(
        LegacySamExtensionService::getInstance()->getLegacySamApiFactory());

    /* Create a card resource extension expecting a SAM "C1" */
    std::shared_ptr<LegacySamSelectionExtension> samSelection(
        legacySamApiFactory->createLegacySamSelectionExtension());
    samSelection
        ->setDynamicUnlockDataProvider(
            std::make_shared<DynamicUnlockDataProvider>())
        .prepareReadAllCountersStatus();

    std::shared_ptr<CardResourceProfileExtension> samCardResourceExtension(
        LegacySamExtensionService::getInstance()
            ->createLegacySamResourceProfileExtension(
                samSelection,
                LegacySamUtil::buildPowerOnDataFilter(
                    LegacySam::ProductType::SAM_C1, "")));

    /* Get the card resource service */
    std::shared_ptr<CardResourceService> cardResourceService(
        CardResourceServiceProvider::getService());

    /* Set up a basic configuration without plugin/reader observation */
    cardResourceService->getConfigurator()
        ->withPlugins(
            PluginsConfigurator::builder()
                ->addPlugin(plugin, std::make_shared<ReaderConfigurator>())
                .build())
        .withCardResourceProfiles(
            {CardResourceProfileConfigurator::builder(
                 SAM_PROFILE_NAME, samCardResourceExtension)
                 ->withReaderNameRegex(ConfigurationUtil::SAM_READER_NAME_REGEX)
                 .build()})
        .configure();
    cardResourceService->start();

    /* Verify if the card resource is available */
    std::shared_ptr<CardResource> cardResource(
        cardResourceService->getCardResource(SAM_PROFILE_NAME));

    if (cardResource == nullptr) {
        throw IllegalStateException(
            "Failed to retrieve a SAM card resource. No card resource found "
            "for profile '"
            + SAM_PROFILE_NAME + "' with reader matching '"
            + ConfigurationUtil::SAM_READER_NAME_REGEX + "' in plugin '"
            + plugin->getName() + "'.");
    }

    /* Release the card resource */
    cardResourceService->releaseCardResource(cardResource);
}

/**
 * Initializes the security settings for the transaction.
 */
static void
initSecuritySetting() {
    std::shared_ptr<CardResource> samResource(
        CardResourceServiceProvider::getService()->getCardResource(
            SAM_PROFILE_NAME));

    symmetricCryptoSecuritySetting
        = calypsoCardApiFactory->createSymmetricCryptoSecuritySetting(
            LegacySamExtensionService::getInstance()
                ->getLegacySamApiFactory()
                ->createSymmetricCryptoCardTransactionManagerFactory(
                    samResource->getReader(),
                    std::dynamic_pointer_cast<LegacySam>(
                        samResource->getSmartCard())));
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
        "= UseCase Calypso #4: Calypso card authentication (Card Resource "
        "Service) ==================\n");

    /* Initialize the context */
    initKeypleService();
    initCalypsoCardExtensionService();
    initCardReader();
    initSamResourceService();
    initSecuritySetting();

    /* Check the card presence */
    if (!cardReader->isCardPresent()) {
        throw IllegalStateException("No card is present in the reader.");
    }

    /* Select the card */
    std::shared_ptr<CalypsoCard> calypsoCard = selectCard(cardReader, AID);

    /*
     * Execute the transaction: the environment file is read within a secure
     * session to ensure data authenticity.
     */
    std::unique_ptr<SecureRegularModeTransactionManagerBase>
        cardTransactionManager(
            calypsoCardApiFactory->createSecureRegularModeTransactionManager(
                cardReader, calypsoCard, symmetricCryptoSecuritySetting));

    dynamic_cast<SecureSymmetricCryptoTransactionManager<
        SecureRegularModeTransactionManagerBase>*>(cardTransactionManager.get())
        ->prepareOpenSecureSession(WriteAccessLevel::DEBIT)
        .prepareReadRecords(SFI_ENVIRONMENT_AND_HOLDER, 1, 1, RECORD_SIZE)
        .prepareCloseSecureSession()
        .processCommands(ChannelControl::CLOSE_AFTER);

    logger->info(
        "The secure session has ended successfully; the card is "
        "authenticated, and the read data is certified.\n");

    const std::string csn(
        HexUtil::toHex(calypsoCard->getApplicationSerialNumber()));
    logger->info("Calypso Serial Number = %\n", csn);

    const std::string sfiEnvHolder(HexUtil::toHex(SFI_ENVIRONMENT_AND_HOLDER));
    logger->info(
        "File SFI %h, rec 1: FILE_CONTENT = %\n",
        sfiEnvHolder,
        calypsoCard->getFileBySfi(SFI_ENVIRONMENT_AND_HOLDER));

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
