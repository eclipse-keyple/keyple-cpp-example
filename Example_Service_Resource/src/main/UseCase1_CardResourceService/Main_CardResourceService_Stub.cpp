/******************************************************************************
 * Copyright (c) 2025 Calypso Networks Association https://calypsonet.org/    *
 *                                                                            *
 * See the NOTICE file(s) distributed with this work for additional           *
 * information regarding copyright ownership.                                 *
 *                                                                            *
 * This program and the accompanying materials are made available under the   *
 * terms of the Eclipse                                                       *
 *                                                                            *
 * Eclipse Distribution License 1.0 which is available at                     *
 * https://www.eclipse.org/org/documents/edl-v10.php                          *
 *                                                                            *
 * SPDX-License-Identifier: BSD-3-Clause                                      *
 ******************************************************************************/

#include <iostream>
#include <memory>
#include <string>

#include "keyple/card/generic/GenericCardSelectionExtension.hpp"
#include "keyple/card/generic/GenericExtensionService.hpp"
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
#include "keyple/core/util/cpp/Thread.hpp"
#include "keyple/core/util/cpp/exception/Exception.hpp"
#include "keyple/plugin/stub/StubPlugin.hpp"
#include "keyple/plugin/stub/StubPluginFactoryBuilder.hpp"
#include "keyple/plugin/stub/StubReader.hpp"
#include "keyple/plugin/stub/StubSmartCard.hpp"
#include "keypop/reader/CardReader.hpp"
#include "keypop/reader/ConfigurableCardReader.hpp"
#include "keypop/reader/ReaderApiFactory.hpp"
#include "keypop/reader/selection/IsoCardSelector.hpp"
#include "keypop/reader/spi/CardReaderObservationExceptionHandlerSpi.hpp"

using keyple::card::generic::GenericCardSelectionExtension;
using keyple::card::generic::GenericExtensionService;
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
using keyple::core::util::cpp::Thread;
using keyple::core::util::cpp::exception::Exception;
using keyple::plugin::stub::StubPlugin;
using keyple::plugin::stub::StubPluginFactoryBuilder;
using keyple::plugin::stub::StubReader;
using keyple::plugin::stub::StubSmartCard;
using keypop::reader::CardReader;
using keypop::reader::ConfigurableCardReader;
using keypop::reader::ReaderApiFactory;
using keypop::reader::selection::IsoCardSelector;
using keypop::reader::spi::CardReaderObservationExceptionHandlerSpi;

/**
 * <h1>Use Case "resource service 1" – Card resource service (Stub)</h1>
 *
 * <p>We demonstrate here the usage of the card resource service with a local
 * pool of Stub readers.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>The card resource service is configured and started to observe the
 *       connection/disconnection of readers and the insertion/removal of cards.
 *   <li>A command line menu allows you to take and release the two defined
 *       types of card resources.
 *   <li>The log and console printouts show the operation of the card resource
 *       service.
 * </ul>
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_CardResourceService_Stub { };
const std::unique_ptr<Logger> logger
    = LoggerFactory::getLogger(typeid(Main_CardResourceService_Stub));

static const std::string READER_A = "READER_A";
static const std::string READER_B = "READER_B";
static const std::string ATR_CARD_A = "3B3F9600805A4880C120501711AABBCC829000";
static const std::string ATR_CARD_B = "3B3F9600805A4880C120501722AABBCC829000";
static const std::string ATR_REGEX_A
    = "^3B3F9600805A4880C120501711[0-9A-F]{6}829000$";
static const std::string ATR_REGEX_B
    = "^3B3F9600805A4880C120501722[0-9A-F]{6}829000$";
static const std::string RESOURCE_A = "RESOURCE_A";
static const std::string RESOURCE_B = "RESOURCE_B";
static const std::string READER_NAME_REGEX_A = ".*_A";
static const std::string READER_NAME_REGEX_B = ".*_B";
static const std::string SAM_PROTOCOL = "ISO_7816_3_T0";

static std::shared_ptr<ReaderApiFactory> readerApiFactory
    = SmartCardServiceProvider::getService()->getReaderApiFactory();

/**
 * Reader configurator used by the card resource service to set up the SAM
 * reader with the required settings.
 */
class ReaderConfigurator : public ReaderConfiguratorSpi {
public:
    /**
     * Constructor.
     */
    ReaderConfigurator() {
    }

    /**
     *
     */
    virtual ~ReaderConfigurator() = default;

    /**
     * {@inheritDoc}
     */
    void
    setupReader(std::shared_ptr<CardReader> reader) override {
        /*
         * Configure the reader with parameters suitable for contactless
         * operations.
         */
        try {
            std::dynamic_pointer_cast<ConfigurableCardReader>(reader)
                ->activateProtocol(SAM_PROTOCOL, SAM_PROTOCOL);

        } catch (const Exception& e) {
            logger->error(
                "Exception raised while setting up the reader %\n",
                reader->getName(),
                e);
        }
    }

private:
    /**
     *
     */
    const std::unique_ptr<Logger> logger
        = LoggerFactory::getLogger(typeid(ReaderConfigurator));
};

/**
 * Class implementing the exception handler SPIs for plugin and reader
 * monitoring.
 */
class PluginAndReaderExceptionHandler
: public PluginObservationExceptionHandlerSpi,
  public CardReaderObservationExceptionHandlerSpi {
public:
    /**
     *
     */
    virtual ~PluginAndReaderExceptionHandler() = default;

    /**
     *
     */
    void
    onPluginObservationError(
        const std::string& pluginName,
        const std::unique_ptr<Exception> e) override {
        logger->error(
            "An exception occurred while monitoring the plugin '%'\n",
            pluginName,
            e->getMessage());
    }

    /**
     *
     */
    void
    onReaderObservationError(
        const std::string& pluginName,
        const std::string& readerName,
        const std::shared_ptr<std::exception> e) override {
        logger->error(
            "An exception occurred while monitoring the reader '%/%' (%)\n",
            pluginName,
            readerName,
            e);
    }
};

/**
 * This method prompts the user to select an option from a menu and returns the
 * selected option as a char.
 *
 * @return The selected option as a char
 */
static char
getInput() {
    std::cout << "Options:" << std::endl;
    std::cout << "    '1': Insert stub card A" << std::endl;
    std::cout << "    '2': Remove stub card A" << std::endl;
    std::cout << "    '3': Insert stub card B" << std::endl;
    std::cout << "    '4': Remove stub card B" << std::endl;
    std::cout << "    '5': Get resource A" << std::endl;
    std::cout << "    '6': Release resource A" << std::endl;
    std::cout << "    '7': Get resource B" << std::endl;
    std::cout << "    '8': Release resource B" << std::endl;
    std::cout << "    'q': quit" << std::endl;
    std::cout << "Select an option: " << std::endl;

    return static_cast<char>(getchar());
}

int
main() {
    /* Get the instance of the SmartCardService (singleton pattern) */
    std::shared_ptr<SmartCardService> smartCardService
        = SmartCardServiceProvider::getService();

    /*
     * Register the StubPlugin with the SmartCardService, get the corresponding
     * generic plugin in return.
     */
    std::shared_ptr<Plugin> plugin = smartCardService->registerPlugin(
        StubPluginFactoryBuilder::builder()->build());

    /* Get the generic card extension service */
    std::shared_ptr<GenericExtensionService> cardExtension
        = GenericExtensionService::getInstance();

    /*
     * Verify that the extension's API level is consistent with the current
     * service
     */
    smartCardService->checkCardExtension(cardExtension);

    logger->info(
        "=============== "
        "UseCase Resource Service #1: card resource service "
        "==================\n");

    /*
     * Create a card resource extension A expecting a card having power-on data
     * matching the regex A.
     */
    std::shared_ptr<IsoCardSelector> cardSelectorA
        = readerApiFactory->createIsoCardSelector();
    cardSelectorA->filterByPowerOnData(ATR_REGEX_A);

    std::shared_ptr<GenericCardSelectionExtension> genericCardSelectionExtension
        = GenericExtensionService::getInstance()
              ->createGenericCardSelectionExtension();

    std::shared_ptr<CardResourceProfileExtension> cardResourceExtensionA
        = GenericExtensionService::getInstance()
              ->createCardResourceProfileExtension(
                  cardSelectorA, genericCardSelectionExtension);

    /*
     * Create a card resource extension B expecting a card having power-on data
     * matching the regex B.
     */
    std::shared_ptr<IsoCardSelector> cardSelectorB
        = readerApiFactory->createIsoCardSelector();
    cardSelectorB->filterByPowerOnData(ATR_REGEX_B);

    std::shared_ptr<CardResourceProfileExtension> cardResourceExtensionB
        = GenericExtensionService::getInstance()
              ->createCardResourceProfileExtension(
                  cardSelectorB, genericCardSelectionExtension);

    /* Get the service */
    std::shared_ptr<CardResourceService> cardResourceService
        = CardResourceServiceProvider::getService();

    auto pluginAndReaderExceptionHandler
        = std::make_shared<PluginAndReaderExceptionHandler>();

    /*
     * Configure the card resource service:
     * - allocation mode is blocking with a 100 milliseconds cycle and a 10
     *   seconds timeout.
     * - the readers are searched in the Stub plugin, the observation of the
     *   plugin (for the connection/disconnection of readers) and of the readers
     *   (for the insertion/removal of cards) is activated.
     * - two card resource profiles A and B are defined, each expecting a
     *   specific card characterized by its power-on data and placed in a
     *   specific reader.
     * - the timeout for using the card's resources is set at 5 seconds.
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
                 RESOURCE_A, cardResourceExtensionA)
                 ->withReaderNameRegex(READER_NAME_REGEX_A)
                 .build(),
             CardResourceProfileConfigurator::builder(
                 RESOURCE_B, cardResourceExtensionB)
                 ->withReaderNameRegex(READER_NAME_REGEX_B)
                 .build()})
        .configure();

    cardResourceService->start();

    std::dynamic_pointer_cast<StubPlugin>(
        plugin->getExtension(typeid(StubPlugin)))
        ->plugReader(READER_A, true, nullptr);
    std::dynamic_pointer_cast<StubPlugin>(
        plugin->getExtension(typeid(StubPlugin)))
        ->plugReader(READER_B, true, nullptr);

    /* Sleep for a moment to let the readers being detected */
    Thread::sleep(2000);

    logger->info(
        "= #### Connect/disconnect readers, insert/remove cards, watch the "
        "log\n");

    bool loop = true;
    std::shared_ptr<CardResource> cardResourceA = nullptr;
    std::shared_ptr<CardResource> cardResourceB = nullptr;

    while (loop) {
        char c = getInput();
        switch (c) {
        case '1':
            std::dynamic_pointer_cast<StubReader>(
                plugin->getReaderExtension(typeid(StubReader), READER_A))
                ->insertCard(
                    StubSmartCard::builder()
                        ->withPowerOnData(HexUtil::toByteArray(ATR_CARD_A))
                        .withProtocol(SAM_PROTOCOL)
                        .build());
            break;
        case '2':
            std::dynamic_pointer_cast<StubReader>(
                plugin->getReaderExtension(typeid(StubReader), READER_A))
                ->removeCard();
            break;
        case '3':
            std::dynamic_pointer_cast<StubReader>(
                plugin->getReaderExtension(typeid(StubReader), READER_B))
                ->insertCard(
                    StubSmartCard::builder()
                        ->withPowerOnData(HexUtil::toByteArray(ATR_CARD_B))
                        .withProtocol(SAM_PROTOCOL)
                        .build());
            break;
        case '4':
            std::dynamic_pointer_cast<StubReader>(
                plugin->getReaderExtension(typeid(StubReader), READER_B))
                ->removeCard();
            break;
        case '5':
            cardResourceA = cardResourceService->getCardResource(RESOURCE_A);
            if (cardResourceA != nullptr) {
                logger->info(
                    "Card resource A is available: reader %, smart card %\n",
                    cardResourceA->getReader()->getName(),
                    cardResourceA->getSmartCard());
            } else {
                logger->info("Card resource A is not available\n");
            }
            break;
        case '6':
            if (cardResourceA != nullptr) {
                logger->info("Release card resource A\n");
                cardResourceService->releaseCardResource(cardResourceA);
            } else {
                logger->error("Card resource A is not available\n");
            }
            break;
        case '7':
            cardResourceB = cardResourceService->getCardResource(RESOURCE_B);
            if (cardResourceB != nullptr) {
                logger->info(
                    "Card resource B is available: reader %, smart card %\n",
                    cardResourceB->getReader()->getName(),
                    cardResourceB->getSmartCard());
            } else {
                logger->info("Card resource B is not available\n");
            }
            break;
        case '8':
            if (cardResourceB != nullptr) {
                logger->info("Release card resource B\n");
                cardResourceService->releaseCardResource(cardResourceB);
            } else {
                logger->error("Card resource B is not available\n");
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

    logger->info("Exit program\n");

    return 0;
}
