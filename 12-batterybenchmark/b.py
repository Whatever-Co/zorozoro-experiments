import asyncio
import platform
import datetime
import time

from bleak import BleakClient

BATTERY_CHARACTERISTIC_UUID = ("10b20108-5b3b-4571-9508-cf3efcd7bbae")
LAMP_CHARACTERISTIC_UUID = ("10b20103-5b3b-4571-9508-cf3efcd7bbae")
MOTOR_CHARACTERISTIC_UUID = ("10b20102-5b3b-4571-9508-cf3efcd7bbae")

battery = 0


def notification_handler(sender, data):
    global battery
    """Simple notification handler which prints the data received."""
    print("{0}: {1}".format(sender, int(data[0])))
    prev = battery
    battery = int(data[0])
    if prev == 0:
        print("battery, {0}, {1}".format(0, battery))


async def run(address, loop):
    async with BleakClient(address, loop=loop) as client:
        x = await client.is_connected()
        #logger.info("Connected: {0}".format(x))
        print("Connected: {0}".format(x))

        # モーター　左を前に100の速度、右を後ろに20の速度
        write_value = bytearray(b'\x01\x01\x01\x3d\x02\x02\x3d')
        # await client.write_gatt_char(MOTOR_CHARACTERISTIC_UUID, write_value)

        await client.start_notify(BATTERY_CHARACTERISTIC_UUID, notification_handler)

        start = time.time()

        while(True):
            # LEDを160ミリ秒、赤に点灯
            write_value = bytearray(b'\x03\x10\x01\x01\xff\x00\x00')
            await client.write_gatt_char(LAMP_CHARACTERISTIC_UUID, write_value)

            await asyncio.sleep(6.0)

            # バッテリー残量読み取り
            battery = await client.read_gatt_char(BATTERY_CHARACTERISTIC_UUID)
            print("battery, {0}, {1}".format(
                int(time.time() - start), int(battery[0])))

if __name__ == "__main__":
    address = (
        # discovery.pyでみつけたtoio Core Cubeのデバイスアドレスをここにセットする
        "D0:8B:7F:12:34:56"  # Windows か Linux のときは16進6バイトのデバイスアドレスを指定
        if platform.system() != "Darwin"
        else "8863A3AA-9773-4D00-B298-8421B2FCFB49"  # macOSのときはmacOSのつけるUUID
    )
    loop = asyncio.get_event_loop()
    loop.run_until_complete(run(address, loop))
