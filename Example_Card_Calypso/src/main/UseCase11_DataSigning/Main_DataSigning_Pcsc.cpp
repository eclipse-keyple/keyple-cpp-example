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
#include <vector>

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
#include "keyple/core/service/spi/PluginObservationExceptionHandlerSpi.hpp"
#include "keyple/core/util/HexUtil.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keyple/core/util/cpp/exception/Exception.hpp"
#include "keyple/core/util/cpp/exception/IllegalStateException.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keypop/calypso/crypto/legacysam/LegacySamApiFactory.hpp"
#include "keypop/calypso/crypto/legacysam/sam/LegacySam.hpp"
#include "keypop/calypso/crypto/legacysam/sam/LegacySamSelectionExtension.hpp"
#include "keypop/calypso/crypto/legacysam/transaction/BasicSignatureComputationData.hpp"
#include "keypop/calypso/crypto/legacysam/transaction/BasicSignatureVerificationData.hpp"
#include "keypop/calypso/crypto/legacysam/transaction/FreeTransactionManager.hpp"
#include "keypop/calypso/crypto/legacysam/transaction/SamTraceabilityMode.hpp"
#include "keypop/calypso/crypto/legacysam/transaction/TraceableSignatureComputationData.hpp"
#include "keypop/calypso/crypto/legacysam/transaction/TraceableSignatureVerificationData.hpp"
#include "keypop/reader/CardReader.hpp"
#include "keypop/reader/ReaderApiFactory.hpp"
#include "keypop/reader/selection/CardSelectionManager.hpp"
#include "keypop/reader/spi/CardReaderObservationExceptionHandlerSpi.hpp"

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
using keyple::core::service::spi::PluginObservationExceptionHandlerSpi;
using keyple::core::util::HexUtil;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keyple::core::util::cpp::exception::Exception;
using keyple::core::util::cpp::exception::IllegalStateException;
using keyple::plugin::pcsc::PcscPluginFactoryBuilder;
using keyple::plugin::pcsc::PcscReader;
using keypop::calypso::crypto::legacysam::LegacySamApiFactory;
using keypop::calypso::crypto::legacysam::sam::LegacySam;
using keypop::calypso::crypto::legacysam::sam::LegacySamSelectionExtension;
using keypop::calypso::crypto::legacysam::transaction::
    BasicSignatureComputationData;
using keypop::calypso::crypto::legacysam::transaction::
    BasicSignatureVerificationData;
using keypop::calypso::crypto::legacysam::transaction::FreeTransactionManager;
using keypop::calypso::crypto::legacysam::transaction::SamTraceabilityMode;
using keypop::calypso::crypto::legacysam::transaction::
    TraceableSignatureComputationData;
using keypop::calypso::crypto::legacysam::transaction::
    TraceableSignatureVerificationData;
using keypop::reader::CardReader;
using keypop::reader::ReaderApiFactory;
using keypop::reader::selection::CardSelectionManager;
using keypop::reader::spi::CardReaderObservationExceptionHandlerSpi;

/**
 * Handles the execution of Calypso Legacy SAM data signing using an SAM
 * resource service and the Legacy SAM extension service.
 *
 * <p>This class demonstrates the process of generating and verifying data
 * signatures with a Calypso Legacy SAM, utilizing a contact reader and the
 * card resource service for Calypso SAM (C1).
 */
class Main_DataSigning_Pcsc { };
static std::unique_ptr<Logger> logger
    = LoggerFactory::getLogger(typeid(Main_DataSigning_Pcsc));

static const std::string SAM_READER_NAME_REGEX = ".*Ident.*";
static const std::uint8_t KIF_BASIC = 0xEC;
static const std::uint8_t KVC_BASIC = 0x85;
static const std::string KIF_BASIC_STR = HexUtil::toHex(KIF_BASIC);
static const std::string KVC_BASIC_STR = HexUtil::toHex(KVC_BASIC);
static const std::uint8_t KIF_TRACEABLE = 0x2B;
static const std::uint8_t KVC_TRACEABLE = 0x19;
static const std::string KIF_TRACEABLE_STR = HexUtil::toHex(KIF_TRACEABLE);
static const std::string KVC_TRACEABLE_STR = HexUtil::toHex(KVC_TRACEABLE);
static const std::string DATA_TO_SIGN = "00112233445566778899AABBCCDDEEFF";

/*
 * The name of the SAM resource provided by the Card Resource Manager and used
 * during the card transaction.
 */
static const std::string SAM_PROFILE_NAME = "SAM C1";

/* The plugin used to manage the readers. */
static std::shared_ptr<Plugin> plugin;
/* The factory used to create the selection manager and card selectors. */
static std::shared_ptr<ReaderApiFactory> readerApiFactory;
/* The Legacy SAM factory used to create the signature data and transaction
 * managers. */
static std::shared_ptr<LegacySamApiFactory> legacySamApiFactory;
/* The Card Resource Service to manage SAM resources. */
static std::shared_ptr<CardResourceService> cardResourceService;

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
 * Class implementing the exception handler SPIs for plugin and reader
 * monitoring.
 */
class PluginAndReaderExceptionHandler final
: public PluginObservationExceptionHandlerSpi,
  public CardReaderObservationExceptionHandlerSpi {
public:
    /**
     * {@inheritDoc}
     */
    void
    onPluginObservationError(
        const std::string& pluginName,
        const std::unique_ptr<Exception> e) override {
        mLogger->error(
            "An exception occurred while monitoring the plugin '%'\n",
            pluginName,
            e->getMessage());
    }

    /**
     * {@inheritDoc}
     */
    void
    onReaderObservationError(
        const std::string& pluginName,
        const std::string& readerName,
        const std::shared_ptr<std::exception> e) override {
        mLogger->error(
            "An exception occurred while monitoring the reader '%/%'\n",
            pluginName,
            readerName,
            e);
    }

private:
    /**
     *
     */
    const std::unique_ptr<Logger> mLogger
        = LoggerFactory::getLogger(typeid(PluginAndReaderExceptionHandler));
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
 * Initializes the Legacy SAM extension service.
 */
static void
initLegacySamExtensionService() {
    std::shared_ptr<LegacySamExtensionService> legacySamExtensionService(
        LegacySamExtensionService::getInstance());
    SmartCardServiceProvider::getService()->checkCardExtension(
        legacySamExtensionService);
    legacySamApiFactory = legacySamExtensionService->getLegacySamApiFactory();
}

/**
 * Initializes the SAM Resource Service making a SAM resource available under
 * the SAM_PROFILE_NAME name.
 */
static void
initSamResourceService() {
    /* Create a card resource extension expecting a SAM "C1" */
    std::shared_ptr<LegacySamSelectionExtension> samSelection(
        legacySamApiFactory->createLegacySamSelectionExtension());

    std::shared_ptr<CardResourceProfileExtension> samCardResourceExtension(
        LegacySamExtensionService::getInstance()
            ->createLegacySamResourceProfileExtension(samSelection));

    /* Get the card resource service */
    cardResourceService = CardResourceServiceProvider::getService();

    auto pluginAndReaderExceptionHandler
        = std::make_shared<PluginAndReaderExceptionHandler>();

    /*
     * Configure the card resource service:
     * - allocation mode is blocking with a 100 milliseconds cycle and a 10
     *   seconds timeout.
     * - the readers are searched in the PC/SC plugin, the observation of the
     *   plugin (for the connection/disconnection of readers) and of the
     *   readers (for the insertion/removal of cards) is activated.
     */
    cardResourceService->getConfigurator()
        ->withBlockingAllocationMode(100, 10000)
        .withPlugins(
            PluginsConfigurator::builder()
                ->addPluginWithMonitoring(
                    plugin,
                    std::make_shared<ReaderConfigurator>(),
                    pluginAndReaderExceptionHandler,
                    pluginAndReaderExceptionHandler)
                .withUsageTimeout(5000)
                .build())
        .withCardResourceProfiles(
            {CardResourceProfileConfigurator::builder(
                 SAM_PROFILE_NAME, samCardResourceExtension)
                 ->withReaderNameRegex(SAM_READER_NAME_REGEX)
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
            + SAM_READER_NAME_REGEX + "' in plugin '" + plugin->getName()
            + "'.");
    }

    /* Release the card resource */
    cardResourceService->releaseCardResource(cardResource);
}

static std::shared_ptr<CardResource>
acquireSamResource(const std::string& cardResourceName) {
    std::shared_ptr<CardResource> cardResource(
        cardResourceService->getCardResource(cardResourceName));
    if (cardResource != nullptr) {
        logger->info(
            "A SAM resource is available: reader %, smart card %\n",
            cardResource->getReader()->getName(),
            cardResource->getSmartCard());
    } else {
        logger->info("SAM resource is not available\n");
    }

    return cardResource;
}

static void
releaseSamResource(std::shared_ptr<CardResource> cardResource) {
    if (cardResource != nullptr) {
        logger->info("Release SAM resource.\n");
        cardResourceService->releaseCardResource(cardResource);
    } else {
        logger->error("SAM resource is not available\n");
    }
}

static void
performBasicSignature(std::shared_ptr<CardResource> cardResource) {
    if (cardResource == nullptr) {
        logger->error("No SAM resource.\n");
        return;
    }

    std::shared_ptr<FreeTransactionManager> freeTransactionManager(
        legacySamApiFactory->createFreeTransactionManager(
            cardResource->getReader(),
            std::dynamic_pointer_cast<LegacySam>(
                cardResource->getSmartCard())));

    logger->info(
        "Signing: data='%' with the key %/%\n",
        DATA_TO_SIGN,
        KIF_BASIC_STR,
        KVC_BASIC_STR);

    std::shared_ptr<BasicSignatureComputationData>
        basicSignatureComputationData(
            legacySamApiFactory->createBasicSignatureComputationData());
    basicSignatureComputationData->setData(
        HexUtil::toByteArray(DATA_TO_SIGN), KIF_BASIC, KVC_BASIC);
    freeTransactionManager->prepareComputeSignature(
        basicSignatureComputationData);
    freeTransactionManager->processCommands();
    const std::string signatureHex(
        HexUtil::toHex(basicSignatureComputationData->getSignature()));
    logger->info("signature='%'\n", signatureHex);

    logger->info(
        "Verifying: data='%', signature='%' with the key %/%\n",
        DATA_TO_SIGN,
        signatureHex,
        KIF_BASIC_STR,
        KVC_BASIC_STR);

    std::shared_ptr<BasicSignatureVerificationData>
        basicSignatureVerificationData(
            legacySamApiFactory->createBasicSignatureVerificationData());
    basicSignatureVerificationData->setData(
        HexUtil::toByteArray(DATA_TO_SIGN),
        HexUtil::toByteArray(signatureHex),
        KIF_BASIC,
        KVC_BASIC);
    freeTransactionManager->prepareVerifySignature(
        basicSignatureVerificationData);
    freeTransactionManager->processCommands();
    const bool isSignatureValid
        = basicSignatureVerificationData->isSignatureValid();
    logger->info("Signature is valid: '%'\n", isSignatureValid);
}

static void
performTraceableSignature(std::shared_ptr<CardResource> cardResource) {
    if (cardResource == nullptr) {
        logger->error("No SAM resource.\n");
        return;
    }

    std::shared_ptr<FreeTransactionManager> freeTransactionManager(
        legacySamApiFactory->createFreeTransactionManager(
            cardResource->getReader(),
            std::dynamic_pointer_cast<LegacySam>(
                cardResource->getSmartCard())));

    logger->info(
        "Signing: data='%' with the key %/%\n",
        DATA_TO_SIGN,
        KIF_TRACEABLE_STR,
        KVC_TRACEABLE_STR);

    std::shared_ptr<TraceableSignatureComputationData>
        traceableSignatureComputationData(
            legacySamApiFactory->createTraceableSignatureComputationData());
    traceableSignatureComputationData
        ->setData(
            HexUtil::toByteArray(DATA_TO_SIGN), KIF_TRACEABLE, KVC_TRACEABLE)
        .withSamTraceabilityMode(0, SamTraceabilityMode::FULL_SERIAL_NUMBER);
    freeTransactionManager->prepareComputeSignature(
        traceableSignatureComputationData);
    freeTransactionManager->processCommands();
    const std::string signatureHex(
        HexUtil::toHex(traceableSignatureComputationData->getSignature()));
    const std::string signedDataHex(
        HexUtil::toHex(traceableSignatureComputationData->getSignedData()));
    logger->info("signature='%'\n", signatureHex);
    logger->info("signed data='%'\n", signedDataHex);

    logger->info(
        "Verifying: data='%', signature='%' with the key %/%\n",
        signedDataHex,
        signatureHex,
        KIF_TRACEABLE_STR,
        KVC_TRACEABLE_STR);

    std::shared_ptr<TraceableSignatureVerificationData>
        traceableSignatureVerificationData(
            legacySamApiFactory->createTraceableSignatureVerificationData());
    traceableSignatureVerificationData
        ->setData(
            HexUtil::toByteArray(signedDataHex),
            HexUtil::toByteArray(signatureHex),
            KIF_TRACEABLE,
            KVC_TRACEABLE)
        .withSamTraceabilityMode(
            0, SamTraceabilityMode::FULL_SERIAL_NUMBER, nullptr);
    freeTransactionManager->prepareVerifySignature(
        traceableSignatureVerificationData);
    freeTransactionManager->processCommands();
    const bool isSignatureValid
        = traceableSignatureVerificationData->isSignatureValid();
    logger->info("Signature is valid: '%'\n", isSignatureValid);
}

static char
getInput() {
    std::cout << "Options:" << std::endl;
    std::cout << "    '1': Get a SAM resource" << std::endl;
    std::cout << "    '2': Release a SAM resource" << std::endl;
    std::cout << "    '3': Basic signature generation and verification"
              << std::endl;
    std::cout << "    '4': Traceable signature generation and verification"
              << std::endl;
    std::cout << "    'q': quit" << std::endl;
    std::cout << "Select an option: " << std::endl;

    return static_cast<char>(getchar());
}

int
main() {
    /* Initialize the context */
    initKeypleService();
    initLegacySamExtensionService();
    initSamResourceService();

    bool loop = true;
    std::shared_ptr<CardResource> cardResource = nullptr;

    while (loop) {
        const char input = getInput();
        switch (input) {
        case '1':
            cardResource = acquireSamResource(SAM_PROFILE_NAME);
            break;
        case '2':
            releaseSamResource(cardResource);
            break;
        case '3':
            performBasicSignature(cardResource);
            break;
        case '4':
            performTraceableSignature(cardResource);
            break;
        case 'q':
            loop = false;
            break;
        default:
            break;
        }
    }

    /* Unregister plugin */
    SmartCardServiceProvider::getService()->unregisterPlugin(plugin->getName());

    logger->info("Exit program.\n");

    return 0;
}
