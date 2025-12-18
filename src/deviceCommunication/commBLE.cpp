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
 * @file commBLE.cpp
 * @authors Gschwind, Weber, Schoch, Niederberger
 *
 * @brief `comm::CommBLE` implementation
 */

#include "commBLE.h"

namespace comm {

CommBLE::CommBLE(DeviceInfo identifier) {
    this->identifier = identifier;
    this->type = ConnType::BLE;
}

CommBLE::~CommBLE() {
    CommBLE::disconnectDevice();
}

bool CommBLE::connectDevice() {
    if (connected) {
        disconnectDevice();
    }

    // Create BLE device info from the device address (stored in identifier.ID)
    QBluetoothDeviceInfo deviceInfo = QBluetoothDeviceInfo(
        QBluetoothAddress(identifier.ID), 
        "", 
        0);

    // Create controller for the BLE device
    bleController = QLowEnergyController::createCentral(deviceInfo);
    
    if (!bleController) {
        qDebug() << "Failed to create BLE controller";
        return false;
    }

    // Connect controller signals
    connect(bleController, &QLowEnergyController::connected, 
            this, &CommBLE::deviceConnected);
    connect(bleController, &QLowEnergyController::disconnected, 
            this, &CommBLE::deviceDisconnected);
    connect(bleController, &QLowEnergyController::serviceDiscovered, 
            this, &CommBLE::serviceDiscovered);
    connect(bleController, &QLowEnergyController::discoveryFinished, 
            this, &CommBLE::serviceScanDone);
    connect(bleController, 
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, &CommBLE::controllerError);

    // Initiate connection
    bleController->connectToDevice();
    
    return true;
}

void CommBLE::disconnectDevice() {
    if (bleService) {
        delete bleService;
        bleService = nullptr;
    }

    if (bleController) {
        bleController->disconnectFromDevice();
        delete bleController;
        bleController = nullptr;
    }

    if (connected) {
        connected = false;
        emit changedStateDevice(connected);
    }
}

void CommBLE::sendData(const QByteArray& rawData) {
    if (!connected || !bleService || !txCharacteristic.isValid()) {
        qDebug() << "Cannot send data: not connected or invalid characteristic";
        return;
    }

    // Write data to the TX characteristic
    bleService->writeCharacteristic(txCharacteristic, rawData);
}

void CommBLE::readData() {
    // Data reading is handled via characteristic notifications
    // The actual data processing is done in updateData()
    
    while (BLEbuffer.length() >= Parser::PACKET_EXPECTED_LEN) {
        if (BLEbuffer[static_cast<int>(Parser::PACKET_EXPECTED_LEN) - 1] == '\r') {
            extractedMessage = BLEbuffer.mid(0, Parser::PACKET_EXPECTED_LEN);
            bool success = parser.parsePackage(extractedMessage, receivedData);
            if (success) {
                emit newSampleDevice(receivedData);
            }
            BLEbuffer.remove(0, Parser::PACKET_EXPECTED_LEN);
        } else {
            BLEbuffer.remove(0, 1);
        }
    }
}

void CommBLE::deviceConnected() {
    qDebug() << "BLE device connected, discovering services...";
    bleController->discoverServices();
}

void CommBLE::deviceDisconnected() {
    qDebug() << "BLE device disconnected";
    connected = false;
    emit changedStateDevice(connected);
}

void CommBLE::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << "Service discovered:" << gatt.toString();
}

void CommBLE::serviceScanDone() {
    qDebug() << "Service scan done";
    
    // Get the LineScale service
    QBluetoothUuid serviceUuid(BLEDefinitions::SERVICE_UUID);
    bleService = bleController->createServiceObject(serviceUuid);
    
    if (!bleService) {
        qDebug() << "LineScale service not found";
        disconnectDevice();
        return;
    }

    // Connect service signals
    connect(bleService, &QLowEnergyService::stateChanged,
            this, &CommBLE::serviceStateChanged);
    connect(bleService, &QLowEnergyService::characteristicChanged,
            this, &CommBLE::updateData);

    // Discover service details
    bleService->discoverDetails();
}

void CommBLE::serviceStateChanged(QLowEnergyService::ServiceState s) {
    if (s == QLowEnergyService::ServiceDiscovered) {
        qDebug() << "Service details discovered";
        
        // Get RX characteristic (device sends data to us)
        QBluetoothUuid rxUuid(BLEDefinitions::CHARACTERISTIC_RX_UUID);
        rxCharacteristic = bleService->characteristic(rxUuid);
        
        // Get TX characteristic (we send data to device)
        QBluetoothUuid txUuid(BLEDefinitions::CHARACTERISTIC_TX_UUID);
        txCharacteristic = bleService->characteristic(txUuid);
        
        if (!rxCharacteristic.isValid() || !txCharacteristic.isValid()) {
            qDebug() << "Invalid characteristics";
            disconnectDevice();
            return;
        }

        // Enable notifications for RX characteristic
        QLowEnergyDescriptor notification = rxCharacteristic.descriptor(
            QBluetoothUuid::ClientCharacteristicConfiguration);
        
        if (notification.isValid()) {
            bleService->writeDescriptor(notification, QByteArray::fromHex("0100"));
        }

        connected = true;
        emit changedStateDevice(connected);
        qDebug() << "BLE connection fully established";
    }
}

void CommBLE::updateData(const QLowEnergyCharacteristic &c, const QByteArray &value) {
    // Receive data from the RX characteristic
    if (c.uuid() == QBluetoothUuid(BLEDefinitions::CHARACTERISTIC_RX_UUID)) {
        BLEbuffer += value;
        readData();
    }
}

void CommBLE::controllerError(QLowEnergyController::Error error) {
    qDebug() << "BLE Controller Error:" << error;
    if (error != QLowEnergyController::NoError) {
        disconnectDevice();
    }
}

}  // namespace comm
