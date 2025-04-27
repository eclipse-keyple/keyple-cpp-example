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

#include <memory>

#include "keyple/card/generic/ChannelControl.hpp"
#include "keyple/card/generic/GenericExtensionService.hpp"
#include "keyple/core/service/Plugin.hpp"
#include "keyple/core/service/SmartCardServiceProvider.hpp"
#include "keyple/core/util/HexUtil.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keyple/core/util/cpp/exception/IllegalStateException.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keyple/plugin/pcsc/PcscSupportedContactlessProtocol.hpp"
#include "keypop/reader/CardReader.hpp"
#include "keypop/reader/ConfigurableCardReader.hpp"
#include "keypop/reader/ReaderApiFactory.hpp"
#include "keypop/reader/selection/CardSelectionResult.hpp"
#include "keypop/reader/selection/spi/SmartCard.hpp"

using keyple::card::generic::ChannelControl;
using keyple::card::generic::GenericExtensionService;
using keyple::core::service::Plugin;
using keyple::core::service::SmartCardServiceProvider;
using keyple::core::util::HexUtil;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keyple::core::util::cpp::exception::IllegalStateException;
using keyple::plugin::pcsc::PcscPluginFactoryBuilder;
using keyple::plugin::pcsc::PcscReader;
using keyple::plugin::pcsc::PcscSupportedContactlessProtocol;
using keypop::reader::CardReader;
using keypop::reader::ConfigurableCardReader;
using keypop::reader::ReaderApiFactory;
using keypop::reader::selection::CardSelectionResult;
using keypop::reader::selection::spi::SmartCard;

/**
 * <h1>Use Case Generic 1 – Basic Selection (PC/SC)</h1>
 *
 * <p>We demonstrate here a selection of cards without any condition related to
 * the card itself. Any card able to communicate with the reader must lead to a
 * "selected" state.<br>
 * Note that in this case, no APDU "select application" is sent to the card.<br>
 * However, upon selection, an APDU command specific to Global Platform
 * compliant cards is sent to the card and may fail depending on the type of
 * card presented.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Check if a ISO 14443-4 card is in the reader, select a card (a
 *       GlobalPlatform compliant card is expected here [e.g. EMV card or
 *       Javacard]).
 *   <li>Run a selection scenario without filter.
 *   <li>Output the collected smart card data (power-on data).
 *   <li>Send a additional APDUs to the card (get Card Production Life Cycle
 *       data [CPLC]).
 * </ul>
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_BasicSelection_Pcsc { };
const std::unique_ptr<Logger>
    logger(LoggerFactory::getLogger(typeid(Main_BasicSelection_Pcsc)));

static std::shared_ptr<Plugin> plugin;
static std::shared_ptr<ReaderApiFactory> readerApiFactory;
static std::shared_ptr<CardReader> cardReader;

static const std::string CONTACTLESS_READER_NAME_REGEX
    = ".*ASK LoGO.*|.*Contactless.*|.*00 01.*";
static const std::string ISO_CARD_PROTOCOL = "ISO_14443_4_CARD";

/**
 * Initializes the Keyple service.
 *
 * <p>Gets an instance of the smart card service, registers the PC/SC plugin,
 * and prepares the reader API factory for use.
 *
 * <p>Retrieves the ReaderApiFactory.
 */
static void
initKeypleService() {
    const auto smartCardService(SmartCardServiceProvider::getService());
    plugin = smartCardService->registerPlugin(
        PcscPluginFactoryBuilder::builder()->build());
    readerApiFactory = smartCardService->getReaderApiFactory();
}

/**
 * Initializes the generic card extension service.
 */
static void
initGenericCardExtensionService() {
    const auto genericExtensionService(GenericExtensionService::getInstance());
    SmartCardServiceProvider::getService()->checkCardExtension(
        genericExtensionService);
}

/**
 * Configures and returns a card reader based on the provided parameters.
 *
 * <p>It finds the reader name by matching with a regular expression, then
 * configures the reader with the specified settings.
 *
 * @param plugin The plugin used to interact with the card reader.
 * @param readerNameRegex The regular expression to match the card reader's
 *        name.
 * @param isContactless A boolean indicating whether the card reader is
 *        contactless.
 * @param isoProtocol The ISO protocol used by the card reader.
 * @param sharingMode The sharing mode of the PC/SC reader.
 * @param physicalProtocolName The name of the protocol used by the reader to
 *        communicate with card.
 * @param logicalProtocolName The name of the protocol known by the application.
 * @return The configured card reader.
 */
static std::shared_ptr<CardReader>
getReader(
    std::shared_ptr<Plugin> plugin,
    const std::string& readerNameRegex,
    const bool isContactless,
    const PcscReader::IsoProtocol isoProtocol,
    const PcscReader::SharingMode sharingMode,
    const std::string& physicalProtocolName,
    const std::string& logicalProtocolName) {
    const auto reader(plugin->findReader(readerNameRegex));

    auto pcscReader(std::dynamic_pointer_cast<PcscReader>(
        plugin->getReaderExtension(typeid(PcscReader), reader->getName())));

    pcscReader->setContactless(isContactless)
        .setIsoProtocol(isoProtocol)
        .setSharingMode(sharingMode);

    std::dynamic_pointer_cast<ConfigurableCardReader>(reader)->activateProtocol(
        physicalProtocolName, logicalProtocolName);

    return reader;
}

/**
 * Initializes the card reader with specific configurations.
 *
 * <p>Prepares the card reader using a predefined set of configurations,
 * including the card reader name regex, ISO protocol, and sharing mode.
 */
static void
initCardReader() {
    cardReader = getReader(
        plugin,
        CONTACTLESS_READER_NAME_REGEX,
        true,
        PcscReader::IsoProtocol::T1,
        PcscReader::SharingMode::EXCLUSIVE,
        PcscSupportedContactlessProtocol::ISO_14443_4.getName(),
        ISO_CARD_PROTOCOL);
}

/**
 * Selects any card for the transaction.
 *
 * <p>Creates a card selection manager, prepares the selection using no filter.
 *
 * @param reader The reader used to communicate with the card.
 * @return The selected smart card ready for the transaction.
 * @throws IllegalStateException if the selection of the application fails.
 */
static std::shared_ptr<SmartCard>
selectCard(std::shared_ptr<CardReader> reader) {
    auto cardSelectionManager(readerApiFactory->createCardSelectionManager());
    auto cardSelector(readerApiFactory->createIsoCardSelector());
    auto genericCardSelectionExtension(
        GenericExtensionService::getInstance()
            ->createGenericCardSelectionExtension());

    cardSelectionManager->prepareSelection(
        cardSelector, genericCardSelectionExtension);

    std::shared_ptr<CardSelectionResult> selectionResult(
        cardSelectionManager->processCardSelectionScenario(reader));

    if (selectionResult->getActiveSmartCard() == nullptr) {
        throw new IllegalStateException("The selection of the card failed.");
    }

    return selectionResult->getActiveSmartCard();
}

int
main() {
    Logger::setLoggerLevel(Logger::Level::logTrace);
    logger->info("= UseCase Generic #1: basic card selection ==============\n");

    initKeypleService();
    initGenericCardExtensionService();
    initCardReader();

    /* Check the card presence */
    if (!cardReader->isCardPresent()) {
        throw IllegalStateException("No card is present in the reader.");
    }

    logger->info("= #### Select the card with no conditions\n");

    std::shared_ptr<SmartCard> smartCard(selectCard(cardReader));

    logger->info("= SmartCard = %\n", smartCard);

    /* Execute an APDU to get CPLC Data (cf. Global Platform Specification) */
    const std::vector<uint8_t> cplcApdu = HexUtil::toByteArray("80CA9F7F00");

    const std::vector<std::string> apduResponses
        = GenericExtensionService::getInstance()
              ->createCardTransaction(cardReader, smartCard)
              ->prepareApdu(cplcApdu)
              .processApdusToHexStrings(ChannelControl::CLOSE_AFTER);

    logger->info("CPLC Data: '%'\n", apduResponses[0]);

    logger->info("= #### End of the generic card processing\n");

    return 0;
}
