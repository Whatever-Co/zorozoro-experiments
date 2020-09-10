EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector:Barrel_Jack J1
U 1 1 5F58C4DA
P 1650 1500
F 0 "J1" H 1707 1825 50  0000 C CNN
F 1 "Barrel_Jack" H 1707 1734 50  0000 C CNN
F 2 "Connector_BarrelJack:BarrelJack_Horizontal" H 1700 1460 50  0001 C CNN
F 3 "~" H 1700 1460 50  0001 C CNN
	1    1650 1500
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT SW1
U 1 1 5F5981F5
P 3050 2900
F 0 "SW1" H 3050 3185 50  0000 C CNN
F 1 "SS12D01G4" H 3050 3094 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 3050 2900 50  0001 C CNN
F 3 "~" H 3050 2900 50  0001 C CNN
	1    3050 2900
	1    0    0    -1  
$EndComp
NoConn ~ 3250 3000
$Comp
L Connector:Conn_01x16_Female C1
U 1 1 5F5B7967
P 2200 2700
F 0 "C1" H 2050 3500 50  0000 L CNN
F 1 "Conn_01x16_Female" H 2228 2585 50  0001 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x16_P2.54mm_Vertical" H 2200 2700 50  0001 C CNN
F 3 "~" H 2200 2700 50  0001 C CNN
	1    2200 2700
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x12_Female C2
U 1 1 5F5BBF14
P 2400 2500
F 0 "C2" H 2292 3093 50  0000 C CNN
F 1 "Conn_01x12_Female" H 2428 2385 50  0001 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x12_P2.54mm_Vertical" H 2400 2500 50  0001 C CNN
F 3 "~" H 2400 2500 50  0001 C CNN
	1    2400 2500
	-1   0    0    -1  
$EndComp
NoConn ~ 2000 2000
NoConn ~ 2000 2100
NoConn ~ 2000 2200
NoConn ~ 2000 2400
NoConn ~ 2000 2500
NoConn ~ 2000 2600
NoConn ~ 2000 2700
NoConn ~ 2000 2800
NoConn ~ 2000 2900
NoConn ~ 2000 3000
NoConn ~ 2000 3100
NoConn ~ 2000 3300
NoConn ~ 2000 3400
NoConn ~ 2000 3500
NoConn ~ 2600 2000
NoConn ~ 2600 2100
NoConn ~ 2600 2300
NoConn ~ 2600 2400
NoConn ~ 2600 2500
NoConn ~ 2600 2600
NoConn ~ 2600 2700
NoConn ~ 2600 2800
NoConn ~ 2600 3000
NoConn ~ 2600 3100
Wire Wire Line
	2600 2900 2850 2900
Wire Wire Line
	1950 1400 2150 1400
Wire Wire Line
	1950 1600 2150 1600
Wire Wire Line
	3250 2800 3500 2800
Text Label 2150 1400 2    50   ~ 0
5V
Text Label 3500 2800 2    50   ~ 0
5V
Text Label 2150 1600 2    50   ~ 0
GND
NoConn ~ 2600 2200
NoConn ~ 2000 2300
Wire Wire Line
	2000 3200 1750 3200
Text Label 1750 3200 2    50   ~ 0
GND
$Comp
L Switch:SW_SPDT SW2
U 1 1 5F59D55C
P 5050 2900
F 0 "SW2" H 5050 3185 50  0000 C CNN
F 1 "SS12D01G4" H 5050 3094 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 5050 2900 50  0001 C CNN
F 3 "~" H 5050 2900 50  0001 C CNN
	1    5050 2900
	1    0    0    -1  
$EndComp
NoConn ~ 5250 3000
$Comp
L Connector:Conn_01x16_Female C3
U 1 1 5F59D563
P 4200 2700
F 0 "C3" H 4050 3500 50  0000 L CNN
F 1 "Conn_01x16_Female" H 4228 2585 50  0001 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x16_P2.54mm_Vertical" H 4200 2700 50  0001 C CNN
F 3 "~" H 4200 2700 50  0001 C CNN
	1    4200 2700
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x12_Female C4
U 1 1 5F59D569
P 4400 2500
F 0 "C4" H 4300 3100 50  0000 C CNN
F 1 "Conn_01x12_Female" H 4428 2385 50  0001 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x12_P2.54mm_Vertical" H 4400 2500 50  0001 C CNN
F 3 "~" H 4400 2500 50  0001 C CNN
	1    4400 2500
	-1   0    0    -1  
$EndComp
NoConn ~ 4000 2000
NoConn ~ 4000 2100
NoConn ~ 4000 2200
NoConn ~ 4000 2400
NoConn ~ 4000 2500
NoConn ~ 4000 2600
NoConn ~ 4000 2700
NoConn ~ 4000 2800
NoConn ~ 4000 2900
NoConn ~ 4000 3000
NoConn ~ 4000 3100
NoConn ~ 4000 3300
NoConn ~ 4000 3400
NoConn ~ 4000 3500
NoConn ~ 4600 2000
NoConn ~ 4600 2100
NoConn ~ 4600 2300
NoConn ~ 4600 2400
NoConn ~ 4600 2500
NoConn ~ 4600 2600
NoConn ~ 4600 2700
NoConn ~ 4600 2800
NoConn ~ 4600 3000
NoConn ~ 4600 3100
Wire Wire Line
	4600 2900 4850 2900
Wire Wire Line
	5250 2800 5500 2800
Text Label 5500 2800 2    50   ~ 0
5V
NoConn ~ 4600 2200
NoConn ~ 4000 2300
Wire Wire Line
	4000 3200 3750 3200
Text Label 3750 3200 2    50   ~ 0
GND
$Comp
L Switch:SW_SPDT SW3
U 1 1 5F5A20B3
P 7050 2900
F 0 "SW3" H 7050 3185 50  0000 C CNN
F 1 "SS12D01G4" H 7050 3094 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 7050 2900 50  0001 C CNN
F 3 "~" H 7050 2900 50  0001 C CNN
	1    7050 2900
	1    0    0    -1  
$EndComp
NoConn ~ 7250 3000
$Comp
L Connector:Conn_01x16_Female C5
U 1 1 5F5A20BA
P 6200 2700
F 0 "C5" H 6050 3500 50  0000 L CNN
F 1 "Conn_01x16_Female" H 6228 2585 50  0001 C TNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x16_P2.54mm_Vertical" H 6200 2700 50  0001 C CNN
F 3 "~" H 6200 2700 50  0001 C CNN
	1    6200 2700
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x12_Female C6
U 1 1 5F5A20C0
P 6400 2500
F 0 "C6" H 6292 3093 50  0000 C CNN
F 1 "Conn_01x12_Female" H 6428 2385 50  0001 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x12_P2.54mm_Vertical" H 6400 2500 50  0001 C CNN
F 3 "~" H 6400 2500 50  0001 C CNN
	1    6400 2500
	-1   0    0    -1  
$EndComp
NoConn ~ 6000 2000
NoConn ~ 6000 2100
NoConn ~ 6000 2200
NoConn ~ 6000 2400
NoConn ~ 6000 2500
NoConn ~ 6000 2600
NoConn ~ 6000 2700
NoConn ~ 6000 2800
NoConn ~ 6000 2900
NoConn ~ 6000 3000
NoConn ~ 6000 3100
NoConn ~ 6000 3300
NoConn ~ 6000 3400
NoConn ~ 6000 3500
NoConn ~ 6600 2000
NoConn ~ 6600 2100
NoConn ~ 6600 2300
NoConn ~ 6600 2400
NoConn ~ 6600 2500
NoConn ~ 6600 2600
NoConn ~ 6600 2700
NoConn ~ 6600 2800
NoConn ~ 6600 3000
NoConn ~ 6600 3100
Wire Wire Line
	6600 2900 6850 2900
Wire Wire Line
	7250 2800 7500 2800
Text Label 7500 2800 2    50   ~ 0
5V
NoConn ~ 6600 2200
NoConn ~ 6000 2300
Wire Wire Line
	6000 3200 5750 3200
Text Label 5750 3200 2    50   ~ 0
GND
$Comp
L Switch:SW_SPDT SW4
U 1 1 5F5D1E2E
P 9050 2900
F 0 "SW4" H 9050 3185 50  0000 C CNN
F 1 "SS12D01G4" H 9050 3094 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 9050 2900 50  0001 C CNN
F 3 "~" H 9050 2900 50  0001 C CNN
	1    9050 2900
	1    0    0    -1  
$EndComp
NoConn ~ 9250 3000
$Comp
L Connector:Conn_01x16_Female C7
U 1 1 5F5D1E35
P 8200 2700
F 0 "C7" H 8050 3500 50  0000 L CNN
F 1 "Conn_01x16_Female" H 8228 2585 50  0001 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x16_P2.54mm_Vertical" H 8200 2700 50  0001 C CNN
F 3 "~" H 8200 2700 50  0001 C CNN
	1    8200 2700
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x12_Female C8
U 1 1 5F5D1E3B
P 8400 2500
F 0 "C8" H 8292 3093 50  0000 C CNN
F 1 "Conn_01x12_Female" H 8428 2385 50  0001 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x12_P2.54mm_Vertical" H 8400 2500 50  0001 C CNN
F 3 "~" H 8400 2500 50  0001 C CNN
	1    8400 2500
	-1   0    0    -1  
$EndComp
NoConn ~ 8000 2000
NoConn ~ 8000 2100
NoConn ~ 8000 2200
NoConn ~ 8000 2400
NoConn ~ 8000 2500
NoConn ~ 8000 2600
NoConn ~ 8000 2700
NoConn ~ 8000 2800
NoConn ~ 8000 2900
NoConn ~ 8000 3000
NoConn ~ 8000 3100
NoConn ~ 8000 3300
NoConn ~ 8000 3400
NoConn ~ 8000 3500
NoConn ~ 8600 2000
NoConn ~ 8600 2100
NoConn ~ 8600 2300
NoConn ~ 8600 2400
NoConn ~ 8600 2500
NoConn ~ 8600 2600
NoConn ~ 8600 2700
NoConn ~ 8600 2800
NoConn ~ 8600 3000
NoConn ~ 8600 3100
Wire Wire Line
	8600 2900 8850 2900
Wire Wire Line
	9250 2800 9500 2800
Text Label 9500 2800 2    50   ~ 0
5V
NoConn ~ 8600 2200
NoConn ~ 8000 2300
Wire Wire Line
	8000 3200 7750 3200
Text Label 7750 3200 2    50   ~ 0
GND
$Comp
L Switch:SW_SPDT SW5
U 1 1 5F5EC239
P 3050 4900
F 0 "SW5" H 3050 5185 50  0000 C CNN
F 1 "SS12D01G4" H 3050 5094 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 3050 4900 50  0001 C CNN
F 3 "~" H 3050 4900 50  0001 C CNN
	1    3050 4900
	1    0    0    -1  
$EndComp
NoConn ~ 3250 5000
$Comp
L Connector:Conn_01x16_Female C9
U 1 1 5F5EC240
P 2200 4700
F 0 "C9" H 2050 5500 50  0000 L CNN
F 1 "Conn_01x16_Female" H 2228 4585 50  0001 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x16_P2.54mm_Vertical" H 2200 4700 50  0001 C CNN
F 3 "~" H 2200 4700 50  0001 C CNN
	1    2200 4700
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x12_Female C10
U 1 1 5F5EC246
P 2400 4500
F 0 "C10" H 2292 5093 50  0000 C CNN
F 1 "Conn_01x12_Female" H 2428 4385 50  0001 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x12_P2.54mm_Vertical" H 2400 4500 50  0001 C CNN
F 3 "~" H 2400 4500 50  0001 C CNN
	1    2400 4500
	-1   0    0    -1  
$EndComp
NoConn ~ 2000 4000
NoConn ~ 2000 4100
NoConn ~ 2000 4200
NoConn ~ 2000 4400
NoConn ~ 2000 4500
NoConn ~ 2000 4600
NoConn ~ 2000 4700
NoConn ~ 2000 4800
NoConn ~ 2000 4900
NoConn ~ 2000 5000
NoConn ~ 2000 5100
NoConn ~ 2000 5300
NoConn ~ 2000 5400
NoConn ~ 2000 5500
NoConn ~ 2600 4000
NoConn ~ 2600 4100
NoConn ~ 2600 4300
NoConn ~ 2600 4400
NoConn ~ 2600 4500
NoConn ~ 2600 4600
NoConn ~ 2600 4700
NoConn ~ 2600 4800
NoConn ~ 2600 5000
NoConn ~ 2600 5100
Wire Wire Line
	2600 4900 2850 4900
Wire Wire Line
	3250 4800 3500 4800
Text Label 3500 4800 2    50   ~ 0
5V
NoConn ~ 2600 4200
NoConn ~ 2000 4300
Wire Wire Line
	2000 5200 1750 5200
Text Label 1750 5200 2    50   ~ 0
GND
$Comp
L Switch:SW_SPDT SW6
U 1 1 5F5EC26B
P 5050 4900
F 0 "SW6" H 5050 5185 50  0000 C CNN
F 1 "SS12D01G4" H 5050 5094 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 5050 4900 50  0001 C CNN
F 3 "~" H 5050 4900 50  0001 C CNN
	1    5050 4900
	1    0    0    -1  
$EndComp
NoConn ~ 5250 5000
$Comp
L Connector:Conn_01x16_Female C11
U 1 1 5F5EC272
P 4200 4700
F 0 "C11" H 4050 5500 50  0000 L CNN
F 1 "Conn_01x16_Female" H 4228 4585 50  0001 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x16_P2.54mm_Vertical" H 4200 4700 50  0001 C CNN
F 3 "~" H 4200 4700 50  0001 C CNN
	1    4200 4700
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x12_Female C12
U 1 1 5F5EC278
P 4400 4500
F 0 "C12" H 4292 5093 50  0000 C CNN
F 1 "Conn_01x12_Female" H 4428 4385 50  0001 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x12_P2.54mm_Vertical" H 4400 4500 50  0001 C CNN
F 3 "~" H 4400 4500 50  0001 C CNN
	1    4400 4500
	-1   0    0    -1  
$EndComp
NoConn ~ 4000 4000
NoConn ~ 4000 4100
NoConn ~ 4000 4200
NoConn ~ 4000 4400
NoConn ~ 4000 4500
NoConn ~ 4000 4600
NoConn ~ 4000 4700
NoConn ~ 4000 4800
NoConn ~ 4000 4900
NoConn ~ 4000 5000
NoConn ~ 4000 5100
NoConn ~ 4000 5300
NoConn ~ 4000 5400
NoConn ~ 4000 5500
NoConn ~ 4600 4000
NoConn ~ 4600 4100
NoConn ~ 4600 4300
NoConn ~ 4600 4400
NoConn ~ 4600 4500
NoConn ~ 4600 4600
NoConn ~ 4600 4700
NoConn ~ 4600 4800
NoConn ~ 4600 5000
NoConn ~ 4600 5100
Wire Wire Line
	4600 4900 4850 4900
Wire Wire Line
	5250 4800 5500 4800
Text Label 5500 4800 2    50   ~ 0
5V
NoConn ~ 4600 4200
NoConn ~ 4000 4300
Wire Wire Line
	4000 5200 3750 5200
Text Label 3750 5200 2    50   ~ 0
GND
$Comp
L Switch:SW_SPDT SW7
U 1 1 5F5EC29D
P 7050 4900
F 0 "SW7" H 7050 5185 50  0000 C CNN
F 1 "SS12D01G4" H 7050 5094 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 7050 4900 50  0001 C CNN
F 3 "~" H 7050 4900 50  0001 C CNN
	1    7050 4900
	1    0    0    -1  
$EndComp
NoConn ~ 7250 5000
$Comp
L Connector:Conn_01x16_Female C13
U 1 1 5F5EC2A4
P 6200 4700
F 0 "C13" H 6050 5500 50  0000 L CNN
F 1 "Conn_01x16_Female" H 6228 4585 50  0001 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x16_P2.54mm_Vertical" H 6200 4700 50  0001 C CNN
F 3 "~" H 6200 4700 50  0001 C CNN
	1    6200 4700
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x12_Female C14
U 1 1 5F5EC2AA
P 6400 4500
F 0 "C14" H 6292 5093 50  0000 C CNN
F 1 "Conn_01x12_Female" H 6428 4385 50  0001 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x12_P2.54mm_Vertical" H 6400 4500 50  0001 C CNN
F 3 "~" H 6400 4500 50  0001 C CNN
	1    6400 4500
	-1   0    0    -1  
$EndComp
NoConn ~ 6000 4000
NoConn ~ 6000 4100
NoConn ~ 6000 4200
NoConn ~ 6000 4400
NoConn ~ 6000 4500
NoConn ~ 6000 4600
NoConn ~ 6000 4700
NoConn ~ 6000 4800
NoConn ~ 6000 4900
NoConn ~ 6000 5000
NoConn ~ 6000 5100
NoConn ~ 6000 5300
NoConn ~ 6000 5400
NoConn ~ 6000 5500
NoConn ~ 6600 4000
NoConn ~ 6600 4100
NoConn ~ 6600 4300
NoConn ~ 6600 4400
NoConn ~ 6600 4500
NoConn ~ 6600 4600
NoConn ~ 6600 4700
NoConn ~ 6600 4800
NoConn ~ 6600 5000
NoConn ~ 6600 5100
Wire Wire Line
	6600 4900 6850 4900
Wire Wire Line
	7250 4800 7500 4800
Text Label 7500 4800 2    50   ~ 0
5V
NoConn ~ 6600 4200
NoConn ~ 6000 4300
Wire Wire Line
	6000 5200 5750 5200
Text Label 5750 5200 2    50   ~ 0
GND
$Comp
L Switch:SW_SPDT SW8
U 1 1 5F5EC2CF
P 9050 4900
F 0 "SW8" H 9050 5185 50  0000 C CNN
F 1 "SS12D01G4" H 9050 5094 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 9050 4900 50  0001 C CNN
F 3 "~" H 9050 4900 50  0001 C CNN
	1    9050 4900
	1    0    0    -1  
$EndComp
NoConn ~ 9250 5000
$Comp
L Connector:Conn_01x16_Female C15
U 1 1 5F5EC2D6
P 8200 4700
F 0 "C15" H 8050 5500 50  0000 L CNN
F 1 "Conn_01x16_Female" H 8228 4585 50  0001 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x16_P2.54mm_Vertical" H 8200 4700 50  0001 C CNN
F 3 "~" H 8200 4700 50  0001 C CNN
	1    8200 4700
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x12_Female C16
U 1 1 5F5EC2DC
P 8400 4500
F 0 "C16" H 8292 5093 50  0000 C CNN
F 1 "Conn_01x12_Female" H 8428 4385 50  0001 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x12_P2.54mm_Vertical" H 8400 4500 50  0001 C CNN
F 3 "~" H 8400 4500 50  0001 C CNN
	1    8400 4500
	-1   0    0    -1  
$EndComp
NoConn ~ 8000 4000
NoConn ~ 8000 4100
NoConn ~ 8000 4200
NoConn ~ 8000 4400
NoConn ~ 8000 4500
NoConn ~ 8000 4600
NoConn ~ 8000 4700
NoConn ~ 8000 4800
NoConn ~ 8000 4900
NoConn ~ 8000 5000
NoConn ~ 8000 5100
NoConn ~ 8000 5300
NoConn ~ 8000 5400
NoConn ~ 8000 5500
NoConn ~ 8600 4000
NoConn ~ 8600 4100
NoConn ~ 8600 4300
NoConn ~ 8600 4400
NoConn ~ 8600 4500
NoConn ~ 8600 4600
NoConn ~ 8600 4700
NoConn ~ 8600 4800
NoConn ~ 8600 5000
NoConn ~ 8600 5100
Wire Wire Line
	8600 4900 8850 4900
Wire Wire Line
	9250 4800 9500 4800
Text Label 9500 4800 2    50   ~ 0
5V
NoConn ~ 8600 4200
NoConn ~ 8000 4300
Wire Wire Line
	8000 5200 7750 5200
Text Label 7750 5200 2    50   ~ 0
GND
$EndSCHEMATC
