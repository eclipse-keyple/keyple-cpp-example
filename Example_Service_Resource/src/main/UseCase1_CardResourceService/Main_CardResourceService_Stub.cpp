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

/* Keyple Core Util */
#include "ByteArrayUtil.h"
#include "ContactCardCommonProtocol.h"
#include "LoggerFactory.h"
#include "Thread.h"

/* Keyple Core Service */
#include "ConfigurableReader.h"
#include "SmartCardService.h"
#include "SmartCardServiceProvider.h"

/* Keyple Service Resource */
#include "CardResourceProfileConfigurator.h"
#include "CardResourceService.h"
#include "CardResourceServiceProvider.h"
#include "PluginsConfigurator.h"

/* Keyple Plugin Stub */
#include "StubPlugin.h"
#include "StubPluginFactoryBuilder.h"
#include "StubReader.h"

/* Keyple Card Generic */
#include "GenericCardSelectionAdapter.h"
#include "GenericExtensionService.h"

/* Keyple Core Common */
#include "KeyplePluginExtension.h"
#include "KeypleReaderExtension.h"

using namespace keyple::card::generic;
using namespace keyple::core::common;
using namespace keyple::core::service;
using namespace keyple::core::service::resource;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp;
using namespace keyple::core::util::protocol;
using namespace keyple::plugin::stub;

/**
 *
 *
 * <h1>Use Case "resource service 1" – Card resource service (Stub)</h1>
 *
 * <p>We demonstrate here the usage of the card resource service with a local pool of Stub readers.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>The card resource service is configured and started to observe the connection/disconnection
 *       of readers and the insertion/removal of cards.
 *   <li>A command line menu allows you to take and release the two defined types of card resources.
 *   <li>The log and console printouts show the operation of the card resource service.
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_CardResourceService_Stub {};
const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_CardResourceService_Stub));

static const std::string READER_A = "READER_A";
static const std::string READER_B = "READER_B";
static const std::string ATR_CARD_A = "3B3F9600805A4880C120501711AABBCC829000";
static const std::string ATR_CARD_B = "3B3F9600805A4880C120501722AABBCC829000";
static const std::string ATR_REGEX_A = "^3B3F9600805A4880C120501711[0-9A-F]{6}829000$";
static const std::string ATR_REGEX_B = "^3B3F9600805A4880C120501722[0-9A-F]{6}829000$";
static const std::string RESOURCE_A = "RESOURCE_A";
static const std::string RESOURCE_B = "RESOURCE_B";
static const std::string READER_NAME_REGEX_A = ".*_A";
static const std::string READER_NAME_REGEX_B = ".*_B";

/**
 * Reader configurator used by the card resource service to setup the SAM reader with the required
 * settings.
 */
class ReaderConfigurator : public ReaderConfiguratorSpi {
public:
    /**
     * Constructor.
     */
    ReaderConfigurator() {}

    /**
     *
     */
    virtual ~ReaderConfigurator() = default;

    /**
     * {@inheritDoc}
     */
    void setupReader(std::shared_ptr<Reader> reader) override {
        /* Configure the reader with parameters suitable for contactless operations */
        try {
            std::dynamic_pointer_cast<ConfigurableReader>(reader)->activateProtocol(
                ContactCardCommonProtocol::ISO_7816_3_T0.getName(),
                ContactCardCommonProtocol::ISO_7816_3_T0.getName());
        } catch (const Exception& e) {
            logger->error("Exception raised while setting up the reader %\n", reader->getName(), e);
        }
    }

private:
    /**
     *
     */
    const std::unique_ptr<Logger> logger = LoggerFactory::getLogger(typeid(ReaderConfigurator));
};

/**
 * Class implementing the exception handler SPIs for plugin and reader monitoring
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
    void onPluginObservationError(const std::string& pluginName, const std::shared_ptr<Exception> e)
        override
    {
        logger->error("An exception occurred while monitoring the plugin '%'\n", pluginName, e);
    }

    /**
     *
     */
    void onReaderObservationError(const std::string& pluginName,
                                  const std::string& readerName,
                                  const std::shared_ptr<Exception> e) override
    {
        logger->error("An exception occurred while monitoring the reader '%/%' (%)\n",
                      pluginName,
                      readerName,
                      e);
    }
};

static char getInput()
{
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

    char key = static_cast<char>(getchar());

    /* Enter key */
    getchar();

    return key;
}

int main()
{
    /* Get the instance of the SmartCardService (singleton pattern) */
    std::shared_ptr<SmartCardService> smartCardService = SmartCardServiceProvider::getService();

    /*
     * Register the StubPlugin with the SmartCardService, get the corresponding generic plugin in
     * return.
     */
    std::shared_ptr<Plugin> plugin =
        smartCardService->registerPlugin(StubPluginFactoryBuilder::builder()->build());

    /* Get the generic card extension service */
    std::shared_ptr<GenericExtensionService> cardExtension = GenericExtensionService::getInstance();

    /* Verify that the extension's API level is consistent with the current service */
    smartCardService->checkCardExtension(cardExtension);

    logger->info("=============== " \
                 "UseCase Resource Service #1: card resource service " \
                 "==================\n");

    /*
     * Create a card resource extension A expecting a card having power-on data matching the regex
     * A.
     */
    std::shared_ptr<GenericCardSelection> cardSelectionA =
        GenericExtensionService::getInstance()->createCardSelection();
    cardSelectionA->filterByPowerOnData(ATR_REGEX_A);

    std::shared_ptr<CardResourceProfileExtension> cardResourceExtensionA =
        GenericExtensionService::getInstance()->createCardResourceProfileExtension(cardSelectionA);

    /*
     * Create a card resource extension B expecting a card having power-on data matching the regex
     * B.
     */
    std::shared_ptr<GenericCardSelection> cardSelectionB =
        GenericExtensionService::getInstance()->createCardSelection();
    cardSelectionB->filterByPowerOnData(ATR_REGEX_B);

    std::shared_ptr<CardResourceProfileExtension> cardResourceExtensionB =
        GenericExtensionService::getInstance()->createCardResourceProfileExtension(cardSelectionB);

    /* Get the service */
    std::shared_ptr<CardResourceService> cardResourceService = CardResourceServiceProvider::getService();

    auto pluginAndReaderExceptionHandler = std::make_shared<PluginAndReaderExceptionHandler>();

    /*
     * Configure the card resource service:
     * - allocation mode is blocking with a 100 milliseconds cycle and a 10 seconds timeout.
     * - the readers are searched in the Stub plugin, the observation of the plugin (for the
     * connection/disconnection of readers) and of the readers (for the insertion/removal of cards)
     * is activated.
     * - two card resource profiles A and B are defined, each expecting a specific card
     * characterized by its power-on data and placed in a specific reader.
     * - the timeout for using the card's resources is set at 5 seconds.
     */
    cardResourceService->getConfigurator()->withBlockingAllocationMode(100, 10000)
                                           .withPlugins(
                                               PluginsConfigurator::builder()
                                                   ->addPluginWithMonitoring(
                                                       plugin,
                                                       std::make_shared<ReaderConfigurator>(),
                                                       pluginAndReaderExceptionHandler,
                                                       pluginAndReaderExceptionHandler)
                                                    .withUsageTimeout(5000)
                                                    .build())
                                           .withCardResourceProfiles({
                                                CardResourceProfileConfigurator::builder(
                                                    RESOURCE_A, cardResourceExtensionA)
                                                    ->withReaderNameRegex(READER_NAME_REGEX_A)
                                                     .build(),
                                                CardResourceProfileConfigurator::builder(
                                                    RESOURCE_B, cardResourceExtensionB)
                                                    ->withReaderNameRegex(READER_NAME_REGEX_B)
                                                     .build()})
                                           .configure();
    cardResourceService->start();

    std::dynamic_pointer_cast<StubPlugin>(plugin->getExtension(typeid(StubPlugin)))
        ->plugReader(READER_A, true, nullptr);
    std::dynamic_pointer_cast<StubPlugin>(plugin->getExtension(typeid(StubPlugin)))
        ->plugReader(READER_B, true, nullptr);

    /* Sleep for a moment to let the readers being detected */
    Thread::sleep(2000);

    std::shared_ptr<Reader> readerA = plugin->getReader(READER_A);
    std::shared_ptr<Reader> readerB = plugin->getReader(READER_B);

    logger->info("= #### Connect/disconnect readers, insert/remove cards, watch the log\n");

    bool loop = true;
    std::shared_ptr<CardResource> cardResourceA = nullptr;
    std::shared_ptr<CardResource> cardResourceB = nullptr;

    while (loop) {
        char c = getInput();
        switch (c) {
        case '1':
            std::dynamic_pointer_cast<StubReader>(readerA->getExtension(typeid(StubReader)))
                ->insertCard(
                    StubSmartCard::builder()
                        ->withPowerOnData(ByteArrayUtil::fromHex(ATR_CARD_A))
                        .withProtocol(ContactCardCommonProtocol::ISO_7816_3_T0.getName())
                        .build());
            break;
        case '2':
            std::dynamic_pointer_cast<StubReader>(readerA->getExtension(typeid(StubReader)))
                ->removeCard();
            break;
        case '3':
            std::dynamic_pointer_cast<StubReader>(readerB->getExtension(typeid(StubReader)))
                ->insertCard(
                    StubSmartCard::builder()
                        ->withPowerOnData(ByteArrayUtil::fromHex(ATR_CARD_B))
                         .withProtocol(ContactCardCommonProtocol::ISO_7816_3_T0.getName())
                         .build());
            break;
        case '4':
            std::dynamic_pointer_cast<StubReader>(readerB->getExtension(typeid(StubReader)))
                ->removeCard();
            break;
        case '5':
            cardResourceA = cardResourceService->getCardResource(RESOURCE_A);
            if (cardResourceA != nullptr) {
                logger->info("Card resource A is available: reader %, smart card %\n",
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
                logger->info("Card resource B is available: reader %, smart card %\n",
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