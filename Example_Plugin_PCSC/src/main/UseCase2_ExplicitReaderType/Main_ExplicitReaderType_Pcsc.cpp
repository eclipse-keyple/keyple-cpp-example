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

#include "keyple/core/service/Plugin.hpp"
#include "keyple/core/service/SmartCardService.hpp"
#include "keyple/core/service/SmartCardServiceProvider.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keypop/reader/CardReader.hpp"

using keyple::core::service::Plugin;
using keyple::core::service::SmartCardService;
using keyple::core::service::SmartCardServiceProvider;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keyple::plugin::pcsc::PcscPluginFactoryBuilder;
using keyple::plugin::pcsc::PcscReader;
using keypop::reader::CardReader;

/**
 * <h1>Use Case PC/SC 2 – Automatic reader type identification (PC/SC)</h1>
 *
 * <p>We demonstrate here how to configure the PC/SC plugin to allow explicit
 * setting of contact/contactless type for a reader.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Configure the plugin (via its factory builder) without specifying
 *       regular expressions.
 *   <li>Set the 'contactless' type for all connected readers.
 *   <li>Display the types of all connected readers.
 * </ul>
 *
 * <p><strong>Note #1:</strong> not all applications need to know what type of
 * reader it is. This parameter is only required if the application or card
 * extension intends to call the CardReader::isContactless() method.
 *
 * <p><strong>Note #2:</strong>: the Keyple Calypso Card extension requires this
 * knowledge.
 *
 * <p><strong>Note #2:</strong>: in a production application, this setting must
 * be applied to the relevant reader.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_ExplicitReaderType_Pcsc { };
static const std::unique_ptr<Logger> logger
    = LoggerFactory::getLogger(typeid(Main_ExplicitReaderType_Pcsc));

int
main() {
    /* Get the instance of the SmartCardService (singleton pattern) */
    std::shared_ptr<SmartCardService> smartCardService
        = SmartCardServiceProvider::getService();

    /*
     * Register the PcscPlugin with the SmartCardService, set the two regular
     * expression matchine the expected devices, get the corresponding generic
     * plugin in return.
     */
    std::shared_ptr<Plugin> plugin = smartCardService->registerPlugin(
        PcscPluginFactoryBuilder::builder()->build());

    /*
     * Set the contactless type to all connected readers through the specific
     * method provided by PC/SC reader's extension.
     */
    for (auto& reader : plugin->getReaders()) {
        std::dynamic_pointer_cast<PcscReader>(
            plugin->getReaderExtension(typeid(PcscReader), reader->getName()))
            ->setContactless(true);
    }

    /* Log the type of each connected reader */
    for (const auto& reader : plugin->getReaders()) {
        logger->info(
            "The reader '%' is a '%' type\n",
            reader->getName(),
            reader->isContactless() ? std::string("contactless")
                                    : std::string("contact"));
    }

    return 0;
}
