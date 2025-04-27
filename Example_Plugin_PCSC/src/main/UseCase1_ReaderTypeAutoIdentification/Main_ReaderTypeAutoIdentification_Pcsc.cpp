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
#include "keypop/reader/CardReader.hpp"

using keyple::core::service::Plugin;
using keyple::core::service::SmartCardService;
using keyple::core::service::SmartCardServiceProvider;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keyple::plugin::pcsc::PcscPluginFactoryBuilder;
using keypop::reader::CardReader;

/**
 * <h1>Use Case PC/SC 1 – Automatic reader type identification (PC/SC)</h1>
 *
 * <p>We demonstrate here how to configure the PC/SC plugin to have an automatic
 * detection of the type of reader (contact/non-contact) from its name.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Configure the plugin (via its factory builder) to specify two regular
 *       expressions to apply to the reader names.
 *   <li>The first regular expression defines the names of readers that are of
 *       the contactless type.
 *   <li>The second regular expression defines the names of readers that are of
 *       the contact type.
 *   <li>Display the types of all connected readers.
 * </ul>
 *
 * <p><strong>Note #1:</strong> not all applications need to know what type of
 *    reader it is. This parameter is only required if the application or card
 *    extension intends to call the CardReader::isContactless() method.
 *
 * <p><strong>Note #2:</strong>: the Keyple Calypso Card extension requires this
 *    knowledge.
 *
 * <p><strong>Note #2:</strong>: In a production application, these regular
 *    expressions must be adapted to the names of the devices used.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_ReaderTypeAutoIdentification_Pcsc { };
static const std::unique_ptr<Logger> logger
    = LoggerFactory::getLogger(typeid(Main_ReaderTypeAutoIdentification_Pcsc));

static const std::string contactlessFilter = ".*ASK LoGO.*|"
                                             ".*HID OMNIKEY 5427 CK.*|"
                                             ".*contactless.*|"
                                             ".*00 01.*|"
                                             ".*5x21-CL 0.*";

static const std::string contactFilter = ".*Identive.*|"
                                         ".*HID Global OMNIKEY 3x21.*|"
                                         "(?=contact)(?!contactless)|"
                                         ".*00 00.*|"
                                         ".*5x21 0.*";

int
main() {
    /* Get the instance of the SmartCardService (singleton pattern) */
    std::shared_ptr<SmartCardService> smartCardService
        = SmartCardServiceProvider::getService();

    /*
     * Register the PcscPlugin with the SmartCardService, set the two regular
     * expression matching the expected devices, get the corresponding generic
     * plugin in return.
     */
    std::shared_ptr<Plugin> plugin = smartCardService->registerPlugin(
        PcscPluginFactoryBuilder::builder()
            ->useContactlessReaderIdentificationFilter(contactlessFilter)
            .useContactReaderIdentificationFilter(contactFilter)
            .build());

    /* Log the type of each reader */
    for (const auto& reader : plugin->getReaders()) {
        logger->info(
            "The reader '%' is a '%' type\n",
            reader->getName(),
            reader->isContactless() ? std::string("contactless")
                                    : std::string("contact"));
    }

    return 0;
}
