from adafruit_ble import BLERadio

from adafruit_ble.advertising import Advertisement
from adafruit_ble.advertising.standard import ProvideServicesAdvertisement
from adafruit_ble.uuid import VendorUUID
from adafruit_ble.services import Service
from adafruit_ble.characteristics import Characteristic, StructCharacteristic
from adafruit_ble.attributes import Attribute

import time
import random


class toioLampCharacteristic(Characteristic):
    uuid = VendorUUID("10B20108-5B3B-4571-9508-CF3EFCD7BBAE")

    def __init__(self):
        super().__init__(
            properties=Characteristic.WRITE,
            read_perm=Attribute.OPEN,
            write_perm=Attribute.OPEN
        )


class toioService(Service):
    uuid = VendorUUID("10B20100-5B3B-4571-9508-CF3EFCD7BBAE")
    lamp = Characteristic(uuid=VendorUUID(
        "10B20103-5B3B-4571-9508-CF3EFCD7BBAE"), properties=Characteristic.WRITE_NO_RESPONSE, read_perm=Attribute.NO_ACCESS)
    battery = StructCharacteristic("B", uuid=VendorUUID(
        "10B20108-5B3B-4571-9508-CF3EFCD7BBAE"))

    def white(self):
        self.lamp = bytearray([0x03, 0, 0x01, 0x01, random.randint(
            0, 255), random.randint(0, 255), random.randint(0, 255)])
        return None


ble = BLERadio()

cube = None
for advertisement in ble.start_scan(ProvideServicesAdvertisement):
    print(advertisement.address)
    if toioService.uuid in advertisement.services:
        print("found toio cube")
        cube = ble.connect(advertisement)
        break

print("scan done")

print(cube)
print(toioService)
service = cube[toioService]
print(service)
print(service.__dict__)
print(service.bleio_service.characteristics)
print(service.battery)
while cube.connected:
    service.white()
    # service.lamp = bytearray([2, 1, 1, 100, 2, 2, 100, 100])
    time.sleep(1)
    print('yes')

print('disconnected...?')
