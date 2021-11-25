# ZOROZORO Experiments

This repository contains the experimental code for the ZOROZORO project, which controls 256 toio cubes simultaneously.

![toio260_1126](https://user-images.githubusercontent.com/27694/143378054-eb7cffa7-544f-4d6e-9711-e092c7f7c5f6.gif)

https://twitter.com/Saqoosha/status/1333233470143774720

## Architecture

![image](https://user-images.githubusercontent.com/27694/143379123-c8bad323-9bec-4a0f-b77d-f4c1f9a01e32.png)

| Module          | Role                                                                                              | Source code                                                                    |
| --------------- | ------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------ |
| Cube Controller | Collects information from the Scanner and Bridge and sends out appropriate commands to each cube. | [`14-cubecontroller`](14-cubecontroller)                                       |
| Bridge          | Make a BLE connection with toio cubes and relay with Controller through TCP socket.               | [`13-unitycontroller2/Arduino/bridge/`](13-unitycontroller2/Arduino/bridge/)   |
| Scanner         | Detects a cube emitting an advertising signal and tells the contoller.                            | [`13-unitycontroller2/Arduino/scanner/`](13-unitycontroller2/Arduino/scanner/) |

## Hardware

- [Adafruit Feather nRF52840 Express](https://www.adafruit.com/product/4062)
- [Adafruit Ethernet FeatherWing](https://www.adafruit.com/product/3201)
- Ethernet Hub
- 5V/10A AC Adapter
- [Custom power distribution board](10-powerpcb/)

![IMG_5856](https://user-images.githubusercontent.com/27694/143379298-fea5e6da-6c5a-4b97-9892-152cacb88424.jpeg)
