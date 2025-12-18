/******************************************************************************
 * Copyright (C) 2022 by Gschwind, Weber, Schoch, Niederberger                *
 *                                                                            *
 * This file is part of linescaleGUI.                                         *
 *                                                                            *
 * LinescaleGUI is free software: you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation, either version 3 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * LinescaleGUI is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with linescaleGUI. If not, see <http://www.gnu.org/licenses/>.       *
 ******************************************************************************/
/**
 * @file commBLE.h
 * @authors Gschwind, Weber, Schoch, Niederberger
 *
 * @brief `comm::CommBLE` declaration
 *
 */

#pragma once
#ifndef COMMBLE_H_
#define COMMBLE_H_

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QDebug>
#include <QObject>
#include "../parser/parser.h"
#include "commDevice.h"

namespace comm {

/**
 * @brief BLE UUIDs for LineScale 3 device communication
 * 
 * These UUIDs define the service and characteristics used for
 * communication with the LineScale 3 device over BLE.
 */
namespace BLEDefinitions {
    // Service UUID for LineScale 3
    // Note: These are placeholder UUIDs and should be updated with actual LineScale 3 UUIDs
    const QString SERVICE_UUID = "{0000ffe0-0000-1000-8000-00805f9b34fb}";
    
    // Characteristic UUID for receiving data from device
    const QString CHARACTERISTIC_RX_UUID = "{0000ffe1-0000-1000-8000-00805f9b34fb}";
    
    // Characteristic UUID for sending data to device
    const QString CHARACTERISTIC_TX_UUID = "{0000ffe2-0000-1000-8000-00805f9b34fb}";
}

/**
 * @brief Class to handle the communication with a BLE device
 *
 */
class CommBLE : public CommDevice {
    Q_OBJECT

   public:
    /**
     * @brief Construct a new Comm BLE object
     *
     * @param identifier Struct with the needed informations about the planned
     * connection
     */
    CommBLE(DeviceInfo identifier);

    /**
     * @brief Disconnect from device and destroy the Comm BLE object
     *
     */
    virtual ~CommBLE();

    /**
     * @brief Implementation for BLE, connect to device as set in the Ctor
     *
     * @return true / false on success or failure of the connection
     */
    bool connectDevice() override;

    /**
     * @brief Disconnect from BLE device
     *
     */
    void disconnectDevice() override;

    /**
     * @brief Send data to connected BLE device
     *
     * @param rawData HEX command with CRC
     */
    void sendData(const QByteArray& rawData) override;

    /**
     * @brief Method to read the received data
     *
     */
    void readData() override;

   private slots:
    /**
     * @brief Slot called when BLE device is connected
     */
    void deviceConnected();

    /**
     * @brief Slot called when BLE device is disconnected
     */
    void deviceDisconnected();

    /**
     * @brief Slot called when service is discovered
     */
    void serviceDiscovered(const QBluetoothUuid &gatt);

    /**
     * @brief Slot called when service discovery is finished
     */
    void serviceScanDone();

    /**
     * @brief Slot called when service state changes
     */
    void serviceStateChanged(QLowEnergyService::ServiceState s);

    /**
     * @brief Slot called when characteristic value changes (data received)
     */
    void updateData(const QLowEnergyCharacteristic &c, const QByteArray &value);

    /**
     * @brief Slot called on controller error
     */
    void controllerError(QLowEnergyController::Error error);

   private:
    DeviceInfo identifier;
    QByteArray BLEbuffer;
    QByteArray extractedMessage;
    
    QLowEnergyController* bleController = nullptr;
    QLowEnergyService* bleService = nullptr;
    QLowEnergyCharacteristic rxCharacteristic;
    QLowEnergyCharacteristic txCharacteristic;
};

}  // namespace comm

#endif  // COMMBLE_H_
