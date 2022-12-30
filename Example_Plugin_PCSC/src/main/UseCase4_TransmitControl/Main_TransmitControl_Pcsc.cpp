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

/* Keyple Core Util */
#include "HexUtil.h"
#include "IllegalStateException.h"
#include "LoggerFactory.h"
#include "StringUtils.h"
#include "Thread.h"

/* Keyple Core Service */
#include "ConfigurableReader.h"
#include "ObservableReader.h"
#include "SmartCardService.h"
#include "SmartCardServiceProvider.h"

/* Keyple Plugin Pcsc */
#include "PcscPluginFactoryBuilder.h"
#include "PcscReader.h"

/* Keyple Card Generic */
#include "GenericExtensionService.h"

/* Keyple Core Common */
#include "KeypleCardExtension.h"

using namespace keyple::card::generic;
using namespace keyple::core::common;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp;
using namespace keyple::core::service;
using namespace keyple::plugin::pcsc;

/**
 * <h1>Use Case PC/SC 4 – Transmit control command to the connected reader</h1>
 *
 * <p>Here we demonstrate how to transmit specific commands to a reader using the Transmit Control
 * mechanism offered by PC/SC.
 *
 * <p>This function of the PC/SC plugin is useful to access specific features of the reader such as
 * setting parameters, controlling LEDs, a buzzer or any other proprietary function defined by the
 * reader manufacturer.
 *
 * <p>Here, we show its use to change the color of the RGB LEDs and activate the buzzer of a
 * SpringCard "Puck One" reader.
 *
 * <h2>Scenario</h2>
 *
 * <ul>
 *   <li>Connect a Puck One reader
 *   <li>Run the program: the LED turns yellow
 *   <li>Present a card that matches the AID: the LED turns green as long as the card is present,
 *       and blue when the card is removed
 *   <li>Present a card that does not match the AID: the LED turns red as long as the card is
 *       present, and blue when the card is removed
 * </ul>
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.1.0
 */
class Main_TransmitControl_Pcsc {};
static const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_TransmitControl_Pcsc));

static const std::string AID = "315449432E49434131";
static const std::vector<uint8_t> CMD_SET_LED_RED = HexUtil::toByteArray("581E010000");
static const std::vector<uint8_t> CMD_SET_LED_GREEN = HexUtil::toByteArray("581E000100");
static const std::vector<uint8_t> CMD_SET_LED_BLUE = HexUtil::toByteArray("581E000001");
static const std::vector<uint8_t> CMD_SET_LED_YELLOW = HexUtil::toByteArray("581E010100");
static const std::vector<uint8_t> CMD_BUZZER_200MS = HexUtil::toByteArray("589300C8");

/** Card observer class. */
class CardObserver
: public CardReaderObserverSpi, public CardReaderObservationExceptionHandlerSpi {
public:
    /**
     * Constructor
     *
     * @param pcscReader The PcscReader is use.
     */
    CardObserver(const std::shared_ptr<PcscReader> pcscReader) : mPcscReader(pcscReader) {}

    /**
     * Changes the LED color depending on the event type.
     *
     * @param event The current event.
     */
    void onReaderEvent(const std::shared_ptr<CardReaderEvent> event) override
    {
        try {
            switch (event->getType()) {
            case CardReaderEvent::CARD_INSERTED:
                mPcscReader->transmitControlCommand(mPcscReader->getIoctlCcidEscapeCommandId(),
                                                    CMD_BUZZER_200MS);
                mPcscReader->transmitControlCommand(mPcscReader->getIoctlCcidEscapeCommandId(),
                                                    CMD_SET_LED_RED);
                break;
            case CardReaderEvent::CARD_MATCHED:
                mPcscReader->transmitControlCommand(mPcscReader->getIoctlCcidEscapeCommandId(),
                                                    CMD_BUZZER_200MS);
                mPcscReader->transmitControlCommand(mPcscReader->getIoctlCcidEscapeCommandId(),
                                                    CMD_SET_LED_GREEN);
                break;
            case CardReaderEvent::CARD_REMOVED:
                mPcscReader->transmitControlCommand(mPcscReader->getIoctlCcidEscapeCommandId(),
                                                    CMD_SET_LED_BLUE);
                break;
            case CardReaderEvent::UNAVAILABLE:
                break;
            }

            /* Finally block */
            if (event->getType() != CardReaderEvent::Type::CARD_REMOVED) {
                /* Indicates the end of the card processing (not needed for a removal event) */
                std::dynamic_pointer_cast<ObservableReader>(SmartCardServiceProvider::getService()
                                                                ->getPlugins()[0]
                                                                ->getReader(event->getReaderName()))
                    ->finalizeCardProcessing();
            }

        } catch (const Exception& e) {

            (void)e;

            /* Finally block */
            if (event->getType() != CardReaderEvent::Type::CARD_REMOVED) {
                /* Indicates the end of the card processing (not needed for a removal event) */
                std::dynamic_pointer_cast<ObservableReader>(SmartCardServiceProvider::getService()
                                                                ->getPlugins()[0]
                                                                ->getReader(event->getReaderName()))
                    ->finalizeCardProcessing();
            }
        }
    }

    void onReaderObservationError(const std::string& pluginName,
                                  const std::string& readerName,
                                  std::shared_ptr<Exception> e) override
    {
        logger->error("An exception occurred in plugin '%', reader '%'\n",
                      pluginName,
                      readerName,
                      e);
    }

private:
    std::shared_ptr<PcscReader> mPcscReader;
};

int main()
{
    /* Get the instance of the SmartCardService (singleton pattern) */
    std::shared_ptr<SmartCardService> smartCardService = SmartCardServiceProvider::getService();

    /*
     * Register the PcscPlugin with the SmartCardService, set the two regular expression matching
     * the expected devices, get the corresponding generic plugin in return.
     */
    std::shared_ptr<Plugin> plugin =
        smartCardService->registerPlugin(PcscPluginFactoryBuilder::builder()->build());

    /* Get the contactless reader (we assume that a SpringCard Puck One reader is connected) */
    std::shared_ptr<ObservableReader> reader = nullptr;
    for (const auto& r : plugin->getReaders()) {
        if (StringUtils::contains(StringUtils::tolower(r->getName()), "contactless")) {
            reader = std::dynamic_pointer_cast<ObservableReader>(r);
        }
    }

    if (reader == nullptr) {
        throw IllegalStateException("Reader not found");
    }

    std::shared_ptr<PcscReader> pcscReader = reader->getExtension(typeid(PcscReader));

    /* Change the LED color to yellow when no card is connected */
    for (int i = 0; i < 3; i++) {
        pcscReader->transmitControlCommand(pcscReader->getIoctlCcidEscapeCommandId(),
                                           CMD_SET_LED_YELLOW);
        Thread::sleep(200);
        pcscReader->transmitControlCommand(pcscReader->getIoctlCcidEscapeCommandId(),
                                           CMD_SET_LED_BLUE);
        Thread::sleep(200);
    }

    /* Get the generic card extension service */
    std::shared_ptr<GenericExtensionService> genericExtensionService =
        GenericExtensionService::getInstance();

    /* Check the extension */
    smartCardService->checkCardExtension(genericExtensionService);

    /* Get the core card selection manager */
    std::unique_ptr<CardSelectionManager> cardSelectionManager =
        smartCardService->createCardSelectionManager();

    /* Create a card selection using the generic card extension. */
    std::shared_ptr<GenericCardSelection> cardSelection =
        genericExtensionService->createCardSelection();
    cardSelection->filterByDfName(AID);

    /* Prepare the selection by adding the created selection to the card selection scenario. */
    cardSelectionManager->prepareSelection(cardSelection);

    /* Schedule the selection scenario, always notify card presence. */
    cardSelectionManager->scheduleCardSelectionScenario(
        reader,
        ObservableCardReader::DetectionMode::REPEATING,
        ObservableCardReader::NotificationMode::ALWAYS);

    auto cardObserver = std::make_shared<CardObserver>(pcscReader);

    reader->setReaderObservationExceptionHandler(cardObserver);
    reader->addObserver(cardObserver);
    reader->startCardDetection(ObservableCardReader::DetectionMode::REPEATING);

    /* Wait indefinitely. CTRL-C to exit. */
    while (1);
}
