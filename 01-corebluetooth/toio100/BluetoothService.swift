//
//  BluetoothService.swift
//  toio100
//
//  Created by Saqoosha on 2020/08/11.
//  Copyright © 2020 Whatever Inc. All rights reserved.
//

import CoreBluetooth

final class BluetoothService: NSObject, CBCentralManagerDelegate {
    private var centralManager: CBCentralManager!
    private var peripherals: [CBPeripheral]!
    private var connected: [CBPeripheral]!
    private var cubes: [toioCoreCube]!

    func startBluetoothScan() {
        centralManager = CBCentralManager(delegate: self, queue: nil)
        peripherals = []
        connected = []
        cubes = []
    }

    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
            case .poweredOff:
                print("Bluetooth PoweredOff")
                break
            case .poweredOn:
                print("Bluetooth poweredOn")
                centralManager.scanForPeripherals(withServices: [CBUUID(string: "10B20100-5B3B-4571-9508-CF3EFCD7BBAE")], options: nil)
                break
            case .resetting:
                print("Bluetooth resetting")
                break
            case .unauthorized:
                print("Bluetooth unauthorized")
                break
            case .unknown:
                print("Bluetooth unknown")
                break
            case .unsupported:
                print("Bluetooth unsupported")
                break
        }
    }

    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String: Any], rssi RSSI: NSNumber) {
        if peripheral.name == "toio Core Cube" && !peripherals.contains(peripheral) {
            print(peripheral)
            print(advertisementData)
//            centralManager.connect(peripheral, options: nil)
//            peripherals.append(peripheral)
        }
    }

    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print("didConnect")
        if connected.contains(peripheral) { return }
        connected.append(peripheral)
        cubes.append(toioCoreCube(peripheral))
    }
}

class toioCoreCube: NSObject, CBPeripheralDelegate {
    var peripheral: CBPeripheral!

    init(_ peripheral: CBPeripheral) {
        super.init()
        self.peripheral = peripheral
        self.peripheral.delegate = self
        self.peripheral.discoverServices([CBUUID(string: "10B20100-5B3B-4571-9508-CF3EFCD7BBAE")])
    }

    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        print("didDiscoverService")
//        print(peripheral.services)
        peripheral.discoverCharacteristics([CBUUID(string: "10B20103-5B3B-4571-9508-CF3EFCD7BBAE")], for: (peripheral.services?.first)!)
    }

    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        print("didDiscoverCharacteristicsFor")
//        print(service.characteristics)
        var data = Data()
        //        data.append(contentsOf: [0x03, 0x64, 0x01, 0x01, 0xFF, 0x00, 0x00]) // Turn on and off the lamp
        data.append(contentsOf: [0x04, 0x00, 0x03,
                                 0x04, 0x01, 0x01, 0xff, 0x00, 0x00,
                                 0x04, 0x01, 0x01, 0x00, 0xff, 0x00,
                                 0x04, 0x01, 0x01, 0x00, 0x00, 0xff])
        peripheral.writeValue(data, for: (service.characteristics?.first)!, type: .withoutResponse)
    }
}
