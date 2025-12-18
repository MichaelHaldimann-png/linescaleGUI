# Communication

## Communication master
Provide a class as an interface to all connected devices. From this interface upwards the type of connection shouldn't matter.

### USB communication
[Qt5 SerialPort](https://doc.qt.io/qt-5/qserialport.html)

SerialPort handling is done with one of the following libraries:

- ```#include <QSerialPort>```
- ```#include <QSerialPortInfo>```

***QSerialPort*** provides all methods to interact with a serial device.  
```QSerialPort::QSerialPort(const QString &name, QObject *parent = nullptr)``` to create a new connection on port ```<name>```  
The baudRate will be set afterwards. 

***QSerialPortInfo*** provides all connected devices; could be used on startup to scan all connected devices. <br>
Get a array of QSerialPortInfo with
```QList<QSerialPortInfo> listOfCOMPorts = QSerialPortInfo::availablePorts();```

### BLE communication
[Qt5 Bluetooth](https://doc.qt.io/qt-5/qtbluetooth-index.html)

BLE handling is done with the following Qt Bluetooth libraries:

- ```#include <QBluetoothDeviceDiscoveryAgent>```
- ```#include <QBluetoothDeviceInfo>```
- ```#include <QLowEnergyController>```
- ```#include <QLowEnergyService>```

***QLowEnergyController*** provides methods to interact with a BLE device.  
```QLowEnergyController::createCentral(const QBluetoothDeviceInfo &remoteDevice, QObject *parent = nullptr)``` creates a new controller for the remote device.

***QLowEnergyService*** provides access to BLE services and characteristics.  
The LineScale 3 uses GATT services and characteristics for data communication.

#### BLE UUIDs
The BLE implementation uses standard UUIDs defined in `commBLE.h`:
- **Service UUID**: Main service for LineScale 3 communication
- **RX Characteristic**: For receiving data from the device  
- **TX Characteristic**: For sending commands to the device

Note: The current UUIDs are placeholders and should be updated with actual LineScale 3 BLE service UUIDs.

#### Connection Flow
1. Create `QLowEnergyController` with device address
2. Connect to device
3. Discover services
4. Get LineScale service
5. Discover characteristics
6. Enable notifications on RX characteristic
7. Ready for bidirectional communication
