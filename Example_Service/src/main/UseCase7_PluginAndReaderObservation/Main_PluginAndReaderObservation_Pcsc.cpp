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

#include "PluginObserver.hpp"

#include "keyple/core/service/ObservablePlugin.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"

using keyple::core::service::ObservablePlugin;
using keyple::plugin::pcsc::PcscPluginFactoryBuilder;

/**
 *
 *
 * <h1>Use Case Generic 7 – plugin and reader observation (PC/SC)</h1>
 *
 * <p>We demonstrate here the monitoring of an {@link ObservablePlugin} to be
 * notified of reader connection/disconnection, and also the monitoring of an
 * ObservableCardReader to be notified of card insertion/removal.
 *
 * <h2>Scenario:</h2>
 *
 * <ul>
 *   <li>Launch the monitoring of the plugin, display potential already
 *       connected reader and already inserted cards.
 *   <li>Display any further reader connection/disconnection or card
 *       insertion/removal.
 *   <li>Automatically observe newly connected readers.
 * </ul>
 *
 * All results are logged with slf4j.
 *
 * <p>Any unexpected behavior will result in runtime exceptions.
 *
 * @since 2.0.0
 */
class Main_PluginAndReaderObservation_Pcsc { };
const std::unique_ptr<Logger> logger
    = LoggerFactory::getLogger(typeid(Main_PluginAndReaderObservation_Pcsc));

int
main() {
    /* Get the instance of the SmartCardService (singleton pattern) */
    std::shared_ptr<SmartCardService> smartCardService(
        SmartCardServiceProvider::getService());

    /*
     * Register the PcscPlugin with the SmartCardService, get the corresponding
     * generic plugin in return.
     */
    std::shared_ptr<ObservablePlugin> plugin
        = std::dynamic_pointer_cast<ObservablePlugin>(
            smartCardService->registerPlugin(
                PcscPluginFactoryBuilder::builder()->build()));

    /*
     * We add an observer to each plugin (only one in this example) the readers
     * observers will be added dynamically upon plugin events notification.
     * Nevertheless, here we provide the plugin observer with the readers
     * already present at startup in order to assign them a reader observer.
     */
    logger->info("Add observer PLUGINNAME = %\n", plugin->getName());
    /* C++: hack */
    // const std::vector<std::shared_ptr<Reader>> readers =
    // plugin->getReaders(); std::vector<std::shared_ptr<CardReader>>
    // cardReaders; for (const auto& reader : readers) {
    //     cardReaders.push_back(std::dynamic_pointer_cast<CardReader>(reader));
    // }
    auto pluginObserver(std::make_shared<PluginObserver>(plugin->getReaders()));
    auto observable(std::dynamic_pointer_cast<ObservablePlugin>(plugin));
    observable->setPluginObservationExceptionHandler(pluginObserver);
    observable->addObserver(pluginObserver);

    logger->info("Wait for reader or card insertion/removal\n");

    while (true);
}
