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
#include "ConfigurableReader.h"
#include "ObservablePlugin.h"
#include "SmartCardService.h"
#include "SmartCardServiceProvider.h"

/* Keyple Plugin Pcsc */
#include "PcscPluginFactoryBuilder.h"

/* Calypsonet Terminal Reader */
#include "CardSelectionManager.h"

/* Examples */
#include "PluginObserver.h"

using namespace calypsonet::terminal::reader::selection;
using namespace keyple::core::service;
using namespace keyple::core::util;
using namespace keyple::core::util::cpp;
using namespace keyple::plugin::pcsc;

/**
 *
 *
 * <h1>Use Case Generic 7 – plugin and reader observation (PC/SC)</h1>
 *
 * <p>We demonstrate here the monitoring of an {@link ObservablePlugin} to be notified of reader
 * connection/disconnection, and also the monitoring of an {@link ObservableReader} to be notified
 * of card insertion/removal.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Launch the monitoring of the plugin, display potential already connected reader and already
 *       inserted cards.
 *   <li>Display any further reader connection/disconnection or card insertion/removal.
 *   <li>Automatically observe newly connected readers.
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_PluginAndReaderObservation_Pcsc {};
const std::unique_ptr<Logger> logger =
    LoggerFactory::getLogger(typeid(Main_PluginAndReaderObservation_Pcsc));

int main()
{
    /* Get the instance of the SmartCardService (singleton pattern) */
    std::shared_ptr<SmartCardService> smartCardService = SmartCardServiceProvider::getService();

    /*
     * Register the PcscPlugin with the SmartCardService, get the corresponding generic plugin in
     * return.
     */
    std::shared_ptr<Plugin> plugin =
        smartCardService->registerPlugin(PcscPluginFactoryBuilder::builder()->build());

    /*
     * We add an observer to each plugin (only one in this example) the readers observers will be
     * added dynamically upon plugin events notification. Nevertheless, here we provide the plugin
     * observer with the readers already present at startup in order to assign them a reader
     * observer.
     */
    logger->info("Add observer PLUGINNAME = %\n", plugin->getName());
    auto pluginObserver = std::make_shared<PluginObserver>(plugin->getReaders());
    auto observable = std::dynamic_pointer_cast<ObservablePlugin>(plugin);
    observable->setPluginObservationExceptionHandler(pluginObserver);
    observable->addObserver(pluginObserver);

    logger->info("Wait for reader or card insertion/removal\n");

    while (true);
}
