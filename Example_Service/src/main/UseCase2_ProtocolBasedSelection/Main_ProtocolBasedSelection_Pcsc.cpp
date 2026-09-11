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

#include <exception>
#include <memory>
#include <string>

#include "../common/ConfigurationUtil.hpp"

#include "keyple/card/generic/GenericExtensionService.hpp"
#include "keyple/core/service/Plugin.hpp"
#include "keyple/core/service/SmartCardServiceProvider.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keyple/core/util/cpp/exception/IllegalStateException.hpp"
#include "keyple/plugin/pcsc/PcscCardCommunicationProtocol.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keyple/plugin/pcsc/PcscSupportedContactlessProtocol.hpp"
#include "keypop/reader/CardReader.hpp"
#include "keypop/reader/ConfigurableCardReader.hpp"
#include "keypop/reader/ReaderApiFactory.hpp"
#include "keypop/reader/selection/CardSelectionResult.hpp"
#include "keypop/reader/selection/spi/SmartCard.hpp"

using keyple::card::generic::GenericExtensionService;
using keyple::core::service::Plugin;
using keyple::core::service::SmartCardServiceProvider;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keyple::core::util::cpp::exception::IllegalStateException;
using keyple::plugin::pcsc::PcscCardCommunicationProtocol;
using keyple::plugin::pcsc::PcscPluginFactoryBuilder;
using keyple::plugin::pcsc::PcscReader;
using keyple::plugin::pcsc::PcscSupportedContactlessProtocol;
using keypop::reader::CardReader;
using keypop::reader::ConfigurableCardReader;
using keypop::reader::ReaderApiFactory;
using keypop::reader::selection::CardSelectionResult;
using keypop::reader::selection::spi::SmartCard;

/**
 * <h1>Use Case Generic 2 – Protocol Based Selection (PC/SC)</h1>
 *
 * <p>We demonstrate here a selection of cards with the only condition being the
 * type of ommunication protocol they use, in this case the Mifare Classic. Any
 * card of the Mifare Classic type must lead to a "selected" status, any card
 * using another protocol must be ignored.<br>
 * Note that in this case, no APDU "select application" is sent to the card.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Check if a ISO 14443-4 card is in the reader, select a card (a Mifare
 *       Classic card is expected here).
 *   <li>Run a selection scenario with the MIFARE CLASSIC protocol filter.
 *   <li>Output the collected smart card data (power-on data).
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_ProtocolBasedSelection_Pcsc { };
const std::unique_ptr<Logger>
    logger(LoggerFactory::getLogger(typeid(Main_ProtocolBasedSelection_Pcsc)));

static std::shared_ptr<Plugin> plugin;
static std::shared_ptr<ReaderApiFactory> readerApiFactory;
static std::shared_ptr<CardReader> cardReader;

static const std::string CONTACTLESS_READER_NAME_REGEX
    = ".*ASK LoGO.*|.*Contactless.*|.*00 01.*";
static const std::string ISO_CARD_PROTOCOL = "ISO_14443_4_CARD";
static const std::string MIFARE_CLASSIC_PROTOCOL = "MIFARE_CLASSIC_CARD";

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
    std::shared_ptr<Plugin> _plugin,
    const std::string& readerNameRegex,
    const bool isContactless,
    const PcscReader::IsoProtocol isoProtocol,
    const PcscReader::SharingMode sharingMode) {
    const auto reader(_plugin->findReader(readerNameRegex));

    if (reader == nullptr) {
        throw IllegalStateException(
            "No reader matching the regex '" + readerNameRegex + "' was found");
    }

    auto pcscReader(std::dynamic_pointer_cast<PcscReader>(
        _plugin->getReaderExtension(typeid(PcscReader), reader->getName())));

    pcscReader->setContactless(isContactless)
        .setIsoProtocol(isoProtocol)
        .setSharingMode(sharingMode);

    std::dynamic_pointer_cast<ConfigurableCardReader>(reader)->activateProtocol(
        PcscCardCommunicationProtocol::ISO_14443_4.getName(),
        ISO_CARD_PROTOCOL);

    std::dynamic_pointer_cast<ConfigurableCardReader>(reader)->activateProtocol(
        PcscSupportedContactlessProtocol::MIFARE_CLASSIC.getName(),
        MIFARE_CLASSIC_PROTOCOL);

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
        PcscReader::SharingMode::EXCLUSIVE);
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
    auto cardSelector(readerApiFactory->createBasicCardSelector());
    cardSelector->filterByCardProtocol(MIFARE_CLASSIC_PROTOCOL);
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

static int
runExample() {
    logger->info("= UseCase Generic #2: protocol based card selection "
                 "==================\n");

    initKeypleService();
    initGenericCardExtensionService();
    initCardReader();

    /* Check if a card is present in the reader */
    if (!cardReader->isCardPresent()) {
        logger->error("No card is present in the reader.\n");
        return 0;
    }

    logger->info(
        "= #### Select the card if the protocol is '%'.\n",
        MIFARE_CLASSIC_PROTOCOL);

    std::shared_ptr<SmartCard> smartCard(selectCard(cardReader));
    if (smartCard == nullptr) {
        logger->error("The selection of the card failed.\n");
        return 0;
    }

    logger->info("= SmartCard = %\n", smartCard);
    logger->info("= #### End of the generic card processing.\n");

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
