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
#include "ConfigurableCardReader.h"

/* Keyple Card Calypso */
#include "CalypsoExtensionService.h"

/* Keyple Core Service */
#include "ConfigurableReader.h"
#include "SmartCardService.h"
#include "SmartCardServiceProvider.h"

/* Keyple Core Util */
#include "HexUtil.h"
#include "IllegalStateException.h"
#include "LoggerFactory.h"
#include "StringUtils.h"

/* Keyple Plugin Pcsc */
#include "PcscPlugin.h"
#include "PcscPluginFactory.h"
#include "PcscPluginFactoryBuilder.h"
#include "PcscReader.h"
#include "PcscSupportedContactProtocol.h"
#include "PcscSupportedContactlessProtocol.h"

/* Keyple Core Resource */
#include "CardResource.h"
#include "CardResourceServiceProvider.h"

/* Keyple Cpp Example */
#include "CalypsoConstants.h"
#include "ConfigurationUtil.h"

using namespace keyple::card::calypso;
using namespace keyple::core::service;
using namespace keyple::core::service::resource;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp;
using namespace keyple::core::util::cpp::exception;
using namespace keyple::plugin::pcsc;

/**
 * <h1>Use Case Calypso 11 – Calypso Card data signing (PC/SC)</h1>
 *
 * <p>We demonstrate here how to generate and verify data signature.
 *
 * <p>Only a contact reader is required for the Calypso SAM.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Sets up the card resource service to provide a Calypso SAM (C1).
 *   <li>The card resource service is configured and started to observe the connection/disconnection
 *       of readers and the insertion/removal of cards.
 *   <li>A command line menu allows you to take and release a SAM resource and select a signature
 *       process.
 *   <li>The log and console printouts show the operation of the card resource service and the
 *       signature processes results.
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 */

class Main_DataSigning_Pcsc{};
const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_DataSigning_Pcsc));

static const std::string SAM_RESOURCE = "SAM_RESOURCE";
static const std::string READER_NAME_REGEX = ".*Ident.*";
static const uint8_t KIF_BASIC = 0xEC;
static const uint8_t KVC_BASIC = 0x85;
static const std::string KIF_BASIC_STR = HexUtil::toHex(KIF_BASIC);
static const std::string KVC_BASIC_STR = HexUtil::toHex(KVC_BASIC);
static const uint8_t KIF_TRACEABLE = 0x2B;
static const uint8_t KVC_TRACEABLE = 0x19;
static const std::string KIF_TRACEABLE_STR = HexUtil::toHex(KIF_TRACEABLE);
static const std::string KVC_TRACEABLE_STR = HexUtil::toHex(KVC_TRACEABLE);
static const std::string DATA_TO_SIGN = "00112233445566778899AABBCCDDEEFF";

/**
 * Reader configurator used by the card resource service to set up the SAM reader with the
 * required settings.
 */
class ReaderConfigurator final : public ReaderConfiguratorSpi {
public:
    /** {@inheritDoc} */
    void setupReader(const std::shared_ptr<CardReader> reader) override
    {
        /* Configure the reader with parameters suitable for contact operations */
        try {
            std::dynamic_pointer_cast<ConfigurableCardReader>(reader)
                ->activateProtocol(PcscSupportedContactProtocol::ISO_7816_3_T0.getName(),
                                   ConfigurationUtil::SAM_PROTOCOL);

            std::shared_ptr<KeypleReaderExtension> readerExtension =
                SmartCardServiceProvider::getService()
                    ->getPlugin(reader)
                    ->getReaderExtension(typeid(KeypleReaderExtension), reader->getName());

            auto pcscReader = std::dynamic_pointer_cast<PcscReader>(readerExtension);
            if (pcscReader) {
                pcscReader->setContactless(false)
                           .setIsoProtocol(PcscReader::IsoProtocol::ANY)
                           .setSharingMode(PcscReader::SharingMode::SHARED);
            }

        } catch (const Exception& e) {
            mLogger->error("Exception raised while setting up the reader %\n",
                          reader->getName(),
                          e);
        }
    }

    /**
     * (private)<br>
     * Constructor.
     */
    ReaderConfigurator() {}

private:
    const std::unique_ptr<Logger> mLogger = LoggerFactory::getLogger(typeid(ReaderConfigurator));
};

/** Class implementing the exception handler SPIs for plugin and reader monitoring. */
class PluginAndReaderExceptionHandler final
: public PluginObservationExceptionHandlerSpi, public CardReaderObservationExceptionHandlerSpi {
public:
    void onPluginObservationError(const std::string& pluginName, const std::shared_ptr<Exception> e)
        override
    {
        logger->error("An exception occurred while monitoring the plugin '%'.\n", pluginName, e);
    }

    void onReaderObservationError(const std::string& pluginName,
                                  const std::string& readerName,
                                  const std::shared_ptr<Exception> e) override
    {
        logger->error("An exception occurred while monitoring the reader '%/%'.\n",
                       pluginName,
                       readerName,
                       e);
    }
};

static char getInput()
{
    std::cout << "Options:" << std::endl;
    std::cout << "    '1': Get a SAM resource" << std::endl;
    std::cout << "    '2': Release a SAM resource" << std::endl;
    std::cout << "    '3': Basic signature generation and verification" << std::endl;
    std::cout << "    '4': Traceable signature generation and verification" << std::endl;
    std::cout << "    'q': quit" << std::endl;
    std::cout << "Select an option: " << std::endl;

    return static_cast<char>(getchar());
}

int main()
{
    /* Get the instance of the SmartCardService */
    std::shared_ptr<SmartCardService> smartCardService = SmartCardServiceProvider::getService();

    /* Register the PcscPlugin, get the corresponding PC/SC plugin in return */
    std::shared_ptr<Plugin> plugin =
        smartCardService->registerPlugin(PcscPluginFactoryBuilder::builder()->build());

    /* Get the Calypso card extension service */
    std::shared_ptr<CalypsoExtensionService> calypsoCardService =
        CalypsoExtensionService::getInstance();

    /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(calypsoCardService);

    /* Create a SAM resource extension expecting a SAM having power-on data matching the regex */
    std::shared_ptr<CalypsoSamSelection> samSelection =
        CalypsoExtensionService::getInstance()->createSamSelection();
    samSelection->filterByProductType(CalypsoSam::ProductType::SAM_C1);

    std::shared_ptr<CardResourceProfileExtension> cardResourceExtension =
        CalypsoExtensionService::getInstance()->createSamResourceProfileExtension(samSelection);

    /* Get the service */
    std::shared_ptr<CardResourceService> cardResourceService =
        CardResourceServiceProvider::getService();

    auto pluginAndReaderExceptionHandler = std::make_shared<PluginAndReaderExceptionHandler>();

    std::shared_ptr<SamTransactionManager> samTransactionManager = nullptr;

    /* Configure the card resource  */
    cardResourceService->getConfigurator()
                       ->withBlockingAllocationMode(100, 10000)
                        .withPlugins(PluginsConfigurator::builder()
                                        ->addPluginWithMonitoring(
                                              plugin,
                                              std::make_shared<ReaderConfigurator>(),
                                              pluginAndReaderExceptionHandler,
                                              pluginAndReaderExceptionHandler)
                                        .withUsageTimeout(5000)
                                        .build())
                        .withCardResourceProfiles(
                            {CardResourceProfileConfigurator::builder(SAM_RESOURCE,
                                                                      cardResourceExtension)
                                 ->withReaderNameRegex(READER_NAME_REGEX)
                                 .build()})
                        .configure();

    cardResourceService->start();

    std::shared_ptr<SamSecuritySetting> samSecuritySetting =
        CalypsoExtensionService::getInstance()->createSamSecuritySetting();

    bool isSignatureValid;
    std::string signatureHex;

    bool loop = true;
    std::shared_ptr<CardResource> cardResource = nullptr;
    while (loop) {
        char c = getInput();
        switch (c) {
        case '1':
            cardResource = cardResourceService->getCardResource(SAM_RESOURCE);
            if (cardResource != nullptr) {
                logger->info("A SAM resource is available: reader %, smart card %\n",
                             cardResource->getReader()->getName(),
                             cardResource->getSmartCard());
            } else {
                logger->info("SAM resource is not available\n");
            }
            break;

        case '2':
            if (cardResource != nullptr) {
                logger->info("Release SAM resource.\n");
                cardResourceService->releaseCardResource(cardResource);
            } else {
                logger->error("SAM resource is not available\n");
            }
            break;

        case '3':
            {
            if (cardResource == nullptr) {
                logger->error("No SAM resource.\n");
                break;
            }
            samTransactionManager =
                CalypsoExtensionService::getInstance()
                    ->createSamTransaction(
                        cardResource->getReader(),
                        std::dynamic_pointer_cast<CalypsoSam>(cardResource->getSmartCard()),
                        samSecuritySetting);

            logger->info("Signing: data='%' with the key %/%\n",
                         DATA_TO_SIGN,
                         KIF_BASIC_STR,
                         KVC_BASIC_STR);

            std::shared_ptr<BasicSignatureComputationData> basicSignatureComputationData =
                CalypsoExtensionService::getInstance()->createBasicSignatureComputationData();
            basicSignatureComputationData->setData(HexUtil::toByteArray(DATA_TO_SIGN),
                                                   KIF_BASIC,
                                                   KVC_BASIC);
            samTransactionManager->prepareComputeSignature(basicSignatureComputationData);
            samTransactionManager->processCommands();
            signatureHex = HexUtil::toHex(basicSignatureComputationData->getSignature());

            logger->info("signature='%'\n", signatureHex);

            logger->info("Verifying: data='%', signature='%' with the key %/%\n",
                         DATA_TO_SIGN,
                         signatureHex,
                         KIF_BASIC_STR,
                         KVC_BASIC_STR);

            std::shared_ptr<BasicSignatureVerificationData> basicSignatureVerificationData =
                CalypsoExtensionService::getInstance()->createBasicSignatureVerificationData();
            basicSignatureVerificationData->setData(HexUtil::toByteArray(DATA_TO_SIGN),
                                                    HexUtil::toByteArray(signatureHex),
                                                    KIF_BASIC,
                                                    KVC_BASIC);
            samTransactionManager->prepareVerifySignature(basicSignatureVerificationData);
            samTransactionManager->processCommands();
            isSignatureValid = basicSignatureVerificationData->isSignatureValid();

            logger->info("Signature is valid: '%'\n", isSignatureValid);
            }
            break;

        case '4':
            {
            if (cardResource == nullptr) {
                logger->error("No SAM resource.");
                break;
            }

            samTransactionManager =
                CalypsoExtensionService::getInstance()
                    ->createSamTransaction(
                        cardResource->getReader(),
                        std::dynamic_pointer_cast<CalypsoSam>(cardResource->getSmartCard()),
                        samSecuritySetting);

            logger->info("Signing: data='%' with the key %/%\n",
                         DATA_TO_SIGN,
                         KIF_TRACEABLE_STR,
                         KVC_TRACEABLE_STR);

            std::shared_ptr<TraceableSignatureComputationData> traceableSignatureComputationData =
                CalypsoExtensionService::getInstance()->createTraceableSignatureComputationData();
            traceableSignatureComputationData->setData(HexUtil::toByteArray(DATA_TO_SIGN),
                                                       KIF_TRACEABLE,
                                                       KVC_TRACEABLE)
                                               .withSamTraceabilityMode(0, true);
            samTransactionManager->prepareComputeSignature(traceableSignatureComputationData);
            samTransactionManager->processCommands();
            signatureHex = HexUtil::toHex(traceableSignatureComputationData->getSignature());
            const std::string signedDataHex =
                HexUtil::toHex(traceableSignatureComputationData->getSignedData());

            logger->info("signature='%'\n", signatureHex);
            logger->info("signed data='%'\n", signedDataHex);

            logger->info("Verifying: data='%', signature='%' with the key %/%\n",
                         signedDataHex,
                         signatureHex,
                         KIF_TRACEABLE_STR,
                         KVC_TRACEABLE_STR);

            std::shared_ptr<TraceableSignatureVerificationData> traceableSignatureVerificationData =
                CalypsoExtensionService::getInstance()->createTraceableSignatureVerificationData();
            traceableSignatureVerificationData->setData(HexUtil::toByteArray(signedDataHex),
                                                        HexUtil::toByteArray(signatureHex),
                                                        KIF_TRACEABLE,
                                                        KVC_TRACEABLE)
                                               .withSamTraceabilityMode(0, true, false);
            samTransactionManager->prepareVerifySignature(traceableSignatureVerificationData);
            samTransactionManager->processCommands();
            isSignatureValid = traceableSignatureVerificationData->isSignatureValid();

            logger->info("Signature is valid: '%'\n", isSignatureValid);
            }
            break;

        case 'q':
            loop = false;
            break;

        default:
            break;
        }
    }

    /* Unregister plugin */
    smartCardService->unregisterPlugin(plugin->getName());

    logger->info("Exit program.\n");
}
