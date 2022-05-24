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
#include "LoggerFactory.h"

/* Keyple Core Service */
#include "SmartCardService.h"
#include "SmartCardServiceProvider.h"

/* Keyple Plugin Pcsc */
#include "PcscPluginFactoryBuilder.h"

using namespace keyple::core::util::cpp;
using namespace keyple::core::service;
using namespace keyple::plugin::pcsc;

/**
 *
 *
 * <h1>Use Case PC/SC 1 – Automatic reader type identification (PC/SC)</h1>
 *
 * <p>We demonstrate here how to configure the PC/SC plugin to have an automatic detection of the
 * type of reader (contact/non-contact) from its name.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Configure the plugin (via its factory builder) to specify two regular expressions to apply
 *       to the reader names.
 *   <li>The first regular expression defines the names of readers that are of the contactless type.
 *   <li>The second regular expression defines the names of readers that are of the contactl type.
 *   <li>Display the types of all connected readers.
 * </ul>
 *
 * <p><strong>Note #1:</strong> not all applications need to know what type of reader it is. This
 * parameter is only required if the application or card extension intends to call the {@link
 * Reader#isContactless()} method.
 *
 * <p><strong>Note #2:</strong>: the Keyple Calypso Card extension requires this knowledge.
 *
 * <p><strong>Note #2:</strong>: In a production application, these regular expressions must be
 * adapted to the names of the devices used.
 *
 * <p>All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_ReaderTypeAutoIdentification_Pcsc {};
static const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_ReaderTypeAutoIdentification_Pcsc));

int main()
{
    /* Get the instance of the SmartCardService (singleton pattern) */
    std::shared_ptr<SmartCardService> smartCardService = SmartCardServiceProvider::getService();

    /*
     * Register the PcscPlugin with the SmartCardService, set the two regular expression matching
     * the expected devices, get the corresponding generic plugin in return.
     */
    std::shared_ptr<Plugin> plugin =
        smartCardService->registerPlugin(
            PcscPluginFactoryBuilder::builder()
                ->useContactlessReaderIdentificationFilter(
                    ".*ASK LoGO.*|.*HID OMNIKEY 5427 CK.*|.*contactless.*|.*00 01.*")
                .useContactReaderIdentificationFilter(
                    ".*Identive.*|.*HID Global OMNIKEY 3x21.*|(?=contact)(?!contactless)|.*00 00.*")
                .build());

    /* Get all connected readers */
    const std::vector<std::shared_ptr<Reader>> readers = plugin->getReaders();

    /* Log the type of each reader */
    for (const auto& reader : readers) {
        logger->info("The reader '%' is a '%' type\n",
            reader->getName(),
            reader->isContactless() ? std::string("contactless") : std::string("contact"));
    }

    return 0;
}
