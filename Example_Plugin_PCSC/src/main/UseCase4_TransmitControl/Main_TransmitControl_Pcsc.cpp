/******************************************************************************
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
#include <vector>

#include "keyple/card/generic/GenericExtensionService.hpp"
#include "keyple/core/service/Plugin.hpp"
#include "keyple/core/service/SmartCardServiceAdapter.hpp"
#include "keyple/core/service/SmartCardServiceProvider.hpp"
#include "keyple/core/util/HexUtil.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keyple/core/util/cpp/StringUtils.hpp"
#include "keyple/core/util/cpp/Thread.hpp"
#include "keyple/core/util/cpp/exception/Exception.hpp"
#include "keyple/core/util/cpp/exception/IllegalStateException.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keypop/reader/CardReaderEvent.hpp"
#include "keypop/reader/ObservableCardReader.hpp"
#include "keypop/reader/ReaderApiFactory.hpp"
#include "keypop/reader/selection/CardSelectionManager.hpp"
#include "keypop/reader/selection/IsoCardSelector.hpp"
#include "keypop/reader/spi/CardReaderObservationExceptionHandlerSpi.hpp"
#include "keypop/reader/spi/CardReaderObserverSpi.hpp"

using keyple::card::generic::GenericExtensionService;
using keyple::core::service::Plugin;
using keyple::core::service::SmartCardServiceAdapter;
using keyple::core::service::SmartCardServiceProvider;
using keyple::core::util::HexUtil;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keyple::core::util::cpp::StringUtils;
using keyple::core::util::cpp::Thread;
using keyple::core::util::cpp::exception::Exception;
using keyple::core::util::cpp::exception::IllegalStateException;
using keyple::plugin::pcsc::PcscPluginFactoryBuilder;
using keyple::plugin::pcsc::PcscReader;
using keypop::reader::CardReaderEvent;
using keypop::reader::ObservableCardReader;
using keypop::reader::ReaderApiFactory;
using keypop::reader::selection::CardSelectionManager;
using keypop::reader::selection::IsoCardSelector;
using keypop::reader::spi::CardReaderObservationExceptionHandlerSpi;
using keypop::reader::spi::CardReaderObserverSpi;

/**
 * <h1>Use Case PC/SC 4 – Transmit control command to the connected reader</h1>
 *
 * <p>Here we demonstrate how to transmit specific commands to a reader using
 * the Transmit Control mechanism offered by PC/SC.
 *
 * <p>This function of the PC/SC plugin is useful to access specific features of
 * the reader such as setting parameters, controlling LEDs, a buzzer or any
 * other proprietary function defined by the reader manufacturer.
 *
 * <p>Here, we show its use to change the color of the RGB LEDs and activate the
 * buzzer of a SpringCard "Puck One" reader.
 *
 * <h2>Scenario</h2>
 *
 * <ul>
 *   <li>Connect a Puck One reader
 *   <li>Run the program: the LED turns yellow
 *   <li>Present a card that matches the AID: the LED turns green as long as the
 *       card is present, and blue when the card is removed
 *   <li>Present a card that does not match the AID: the LED turns red as long
 *       as the card is present, and blue when the card is removed
 * </ul>
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.1.0
 */
class Main_TransmitControl_Pcsc { };
static const std::unique_ptr<Logger> logger
    = LoggerFactory::getLogger(typeid(Main_TransmitControl_Pcsc));

static const std::string AID = "315449432E49434131";
static const std::vector<uint8_t> CMD_SET_LED_RED
    = HexUtil::toByteArray("581E010000");
static const std::vector<uint8_t> CMD_SET_LED_GREEN
    = HexUtil::toByteArray("581E000100");
static const std::vector<uint8_t> CMD_SET_LED_BLUE
    = HexUtil::toByteArray("581E000001");
static const std::vector<uint8_t> CMD_SET_LED_YELLOW
    = HexUtil::toByteArray("581E010100");
static const std::vector<uint8_t> CMD_BUZZER_200MS
    = HexUtil::toByteArray("589300C8");

/** Card observer class. */
class CardObserver final : public CardReaderObserverSpi,
                           public CardReaderObservationExceptionHandlerSpi {
public:
    /**
     * Constructor
     *
     * @param pcscReader The PcscReader is use.
     */
    explicit CardObserver(const std::shared_ptr<PcscReader> pcscReader)
    : mPcscReader(pcscReader) {
    }

    /**
     * Changes the LED color depending on the event type.
     *
     * @param event The current event.
     */
    void
    onReaderEvent(const std::shared_ptr<CardReaderEvent> event) override {
        try {
            switch (event->getType()) {
            case CardReaderEvent::CARD_INSERTED:
                mPcscReader->transmitControlCommand(
                    mPcscReader->getIoctlCcidEscapeCommandId(),
                    CMD_BUZZER_200MS);
                mPcscReader->transmitControlCommand(
                    mPcscReader->getIoctlCcidEscapeCommandId(),
                    CMD_SET_LED_RED);
                break;
            case CardReaderEvent::CARD_MATCHED:
                mPcscReader->transmitControlCommand(
                    mPcscReader->getIoctlCcidEscapeCommandId(),
                    CMD_BUZZER_200MS);
                mPcscReader->transmitControlCommand(
                    mPcscReader->getIoctlCcidEscapeCommandId(),
                    CMD_SET_LED_GREEN);
                break;
            case CardReaderEvent::CARD_REMOVED:
                mPcscReader->transmitControlCommand(
                    mPcscReader->getIoctlCcidEscapeCommandId(),
                    CMD_SET_LED_BLUE);
                break;
            case CardReaderEvent::UNAVAILABLE:
                break;
            }

            /* Finally block */
            if (event->getType() != CardReaderEvent::Type::CARD_REMOVED) {
                /*
                 * Indicates the end of the card processing (not needed for a
                 * removal event).
                 */
                std::dynamic_pointer_cast<ObservableCardReader>(
                    SmartCardServiceProvider::getService()
                        ->getPlugins()[0]
                        ->getReader(event->getReaderName()))
                    ->finalizeCardProcessing();
            }

        } catch (const Exception&) {
            /* Finally block */
            if (event->getType() != CardReaderEvent::Type::CARD_REMOVED) {
                /*
                 * Indicates the end of the card processing (not needed for a
                 * removal event).
                 */
                std::dynamic_pointer_cast<ObservableCardReader>(
                    SmartCardServiceProvider::getService()
                        ->getPlugins()[0]
                        ->getReader(event->getReaderName()))
                    ->finalizeCardProcessing();
            }
        }
    }

    void
    onReaderObservationError(
        const std::string& pluginName,
        const std::string& readerName,
        const std::shared_ptr<std::exception> e) override {
        logger->error(
            "An exception occurred in plugin '%', reader '%'\n",
            pluginName,
            readerName,
            e);
    }

private:
    std::shared_ptr<PcscReader> mPcscReader;
};

int
main() {
    /* Get the instance of the SmartCardService (singleton pattern) */
    std::shared_ptr<SmartCardServiceAdapter> smartCardService(
        SmartCardServiceProvider::getService());

    /*
     * Register the PcscPlugin with the SmartCardService, set the two regular
     * expression matching the expected devices, get the corresponding generic
     * plugin in return.
     */
    std::shared_ptr<Plugin> plugin(smartCardService->registerPlugin(
        PcscPluginFactoryBuilder::builder()->build()));

    /*
     * Get the contactless reader (we assume that a SpringCard Puck One reader
     * is connected).
     */
    std::shared_ptr<ObservableCardReader> reader = nullptr;
    for (const auto& r : plugin->getReaders()) {
        if (StringUtils::contains(
                StringUtils::tolower(r->getName()), "contactless")) {
            reader = std::dynamic_pointer_cast<ObservableCardReader>(r);
        }
    }

    if (reader == nullptr) {
        throw IllegalStateException("Reader not found");
    }

    std::shared_ptr<PcscReader> pcscReader(
        std::dynamic_pointer_cast<PcscReader>(
            plugin->getReaderExtension(typeid(PcscReader), reader->getName())));

    /* Change the LED color to yellow when no card is connected */
    for (int i = 0; i < 3; i++) {
        pcscReader->transmitControlCommand(
            pcscReader->getIoctlCcidEscapeCommandId(), CMD_SET_LED_YELLOW);
        Thread::sleep(200);
        pcscReader->transmitControlCommand(
            pcscReader->getIoctlCcidEscapeCommandId(), CMD_SET_LED_BLUE);
        Thread::sleep(200);
    }

    /* Get the generic card extension service */
    std::shared_ptr<GenericExtensionService> cardExtension
        = GenericExtensionService::getInstance();

    /* Check the extension */
    smartCardService->checkCardExtension(cardExtension);

    /* Retrieve the reader API factory. */
    std::shared_ptr<ReaderApiFactory> readerApiFactory(
        smartCardService->getReaderApiFactory());

    /* Get the core card selection manager */
    std::shared_ptr<CardSelectionManager> cardSelectionManager(
        readerApiFactory->createCardSelectionManager());

    /*
     * Create a card selection using the generic card extension without
     * specifying any filter (protocol/ATR/DFName).
     */
    std::shared_ptr<IsoCardSelector> cardSelector(
        readerApiFactory->createIsoCardSelector());
    cardSelector->filterByDfName(AID);

    /*
     * Prepare the selection by adding the created generic selection to the card
     * selection scenario.
     */
    cardSelectionManager->prepareSelection(
        cardSelector, cardExtension->createGenericCardSelectionExtension());

    /* Schedule the selection scenario, always notify card presence. */
    cardSelectionManager->scheduleCardSelectionScenario(
        reader, ObservableCardReader::NotificationMode::ALWAYS);

    auto cardObserver(std::make_shared<CardObserver>(pcscReader));

    reader->setReaderObservationExceptionHandler(cardObserver);
    reader->addObserver(cardObserver);
    reader->startCardDetection(ObservableCardReader::DetectionMode::REPEATING);

    /* Wait indefinitely. CTRL-C to exit. */
    while (1) {
    }
}
