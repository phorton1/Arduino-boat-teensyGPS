EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A2 23386 16535
encoding utf-8
Sheet 1 1
Title "BOARD2 - on Rhapsody"
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text GLabel 1600 5800 3    50   Input ~ 0
CTX
Text GLabel 2150 1300 3    50   Input ~ 0
CANH
Text GLabel 2050 1300 3    50   Input ~ 0
CANL
Text GLabel 1800 5800 3    50   Input ~ 0
CANL
Text GLabel 1700 5800 3    50   Input ~ 0
CANH
Text GLabel 1500 5800 3    50   Input ~ 0
CRX
$Comp
L power:GND #PWR022
U 1 1 68C8AF7F
P 1400 5800
F 0 "#PWR022" H 1400 5550 50  0001 C CNN
F 1 "GND" V 1400 5600 50  0000 C CNN
F 2 "" H 1400 5800 50  0001 C CNN
F 3 "" H 1400 5800 50  0001 C CNN
	1    1400 5800
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR021
U 1 1 68C85564
P 1300 5800
F 0 "#PWR021" H 1300 5650 50  0001 C CNN
F 1 "+3V3" V 1300 6000 50  0000 C CNN
F 2 "" H 1300 5800 50  0001 C CNN
F 3 "" H 1300 5800 50  0001 C CNN
	1    1300 5800
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x06 J6
U 1 1 68C7D765
P 1500 5600
F 0 "J6" V 1650 5750 50  0000 L CNN
F 1 "SN65HVD230" V 1650 5200 50  0000 L CNN
F 2 "0_my_footprints:CANBUS_Module1" H 1500 5600 50  0001 C CNN
F 3 "~" H 1500 5600 50  0001 C CNN
	1    1500 5600
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR015
U 1 1 68BD4022
P 8900 3850
F 0 "#PWR015" H 8900 3600 50  0001 C CNN
F 1 "GND" H 8905 3677 50  0000 C CNN
F 2 "" H 8900 3850 50  0001 C CNN
F 3 "" H 8900 3850 50  0001 C CNN
	1    8900 3850
	1    0    0    -1  
$EndComp
Wire Wire Line
	7550 3750 8050 3750
Wire Wire Line
	7900 3250 8900 3250
Wire Wire Line
	7900 3050 7900 3250
Wire Wire Line
	8800 2850 8800 2400
Wire Wire Line
	7550 3550 7550 3450
Wire Wire Line
	7650 3550 7550 3550
Wire Wire Line
	8050 3550 7950 3550
$Comp
L Device:R R2
U 1 1 68211A3C
P 8950 3050
F 0 "R2" H 9020 3096 50  0001 L CNN
F 1 "4.7K" V 8950 3050 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 8880 3050 50  0001 C CNN
F 3 "~" H 8950 3050 50  0001 C CNN
	1    8950 3050
	0    -1   -1   0   
$EndComp
$Comp
L power:+3V3 #PWR014
U 1 1 68244B74
P 7550 3450
F 0 "#PWR014" H 7550 3300 50  0001 C CNN
F 1 "+3V3" H 7565 3623 50  0000 C CNN
F 2 "" H 7550 3450 50  0001 C CNN
F 3 "" H 7550 3450 50  0001 C CNN
	1    7550 3450
	1    0    0    -1  
$EndComp
$Comp
L Device:R R3
U 1 1 68236F3F
P 7800 3550
F 0 "R3" H 7870 3596 50  0001 L CNN
F 1 "220" V 7800 3550 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 7730 3550 50  0001 C CNN
F 3 "~" H 7800 3550 50  0001 C CNN
	1    7800 3550
	0    1    1    0   
$EndComp
$Comp
L Isolator:TLP291 U3
U 1 1 68235388
P 8350 3650
F 0 "U3" H 8350 3975 50  0001 C CNN
F 1 "TLP521" H 8350 3883 50  0000 C CNN
F 2 "0_my_footprints:myDip4" H 8150 3450 50  0001 L CIN
F 3 "https://toshiba.semicon-storage.com/info/docget.jsp?did=12884&prodName=TLP291" H 8350 3650 50  0001 L CNN
	1    8350 3650
	1    0    0    -1  
$EndComp
$Comp
L Isolator:TLP291 U1
U 1 1 6822E37B
P 8350 2950
F 0 "U1" H 8350 3275 50  0001 C CNN
F 1 "TLP521" H 8350 3183 50  0000 C CNN
F 2 "0_my_footprints:myDip4" H 8150 2750 50  0001 L CIN
F 3 "https://toshiba.semicon-storage.com/info/docget.jsp?did=12884&prodName=TLP291" H 8350 2950 50  0001 L CNN
	1    8350 2950
	-1   0    0    -1  
$EndComp
Connection ~ 7900 2850
Wire Wire Line
	7550 2850 7900 2850
Wire Wire Line
	7900 2850 7900 2700
Wire Wire Line
	8050 2850 7900 2850
Wire Wire Line
	8050 3050 7900 3050
Wire Wire Line
	8650 2850 8800 2850
Wire Wire Line
	8800 3050 8650 3050
$Comp
L power:+12V #PWR09
U 1 1 6820AAC2
P 8800 2400
F 0 "#PWR09" H 8800 2250 50  0001 C CNN
F 1 "+12V" H 8815 2573 50  0000 C CNN
F 2 "" H 8800 2400 50  0001 C CNN
F 3 "" H 8800 2400 50  0001 C CNN
	1    8800 2400
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR08
U 1 1 681EA407
P 7900 2400
F 0 "#PWR08" H 7900 2250 50  0001 C CNN
F 1 "+3V3" H 7915 2573 50  0000 C CNN
F 2 "" H 7900 2400 50  0001 C CNN
F 3 "" H 7900 2400 50  0001 C CNN
	1    7900 2400
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C1
U 1 1 68D022FD
P 2950 3050
F 0 "C1" H 3068 3096 50  0001 L CNN
F 1 "100uf" H 2600 3050 50  0000 L CNN
F 2 "0_my_footprints2:CP_my100uf" H 2988 2900 50  0001 C CNN
F 3 "~" H 2950 3050 50  0001 C CNN
	1    2950 3050
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x04 J1
U 1 1 68DEB69F
P 2050 1100
F 0 "J1" V 2200 1200 50  0000 L CNN
F 1 "NMEA2000" V 2200 750 50  0000 L CNN
F 2 "0_my_footprints2:JST4" H 2050 1100 50  0001 C CNN
F 3 "~" H 2050 1100 50  0001 C CNN
	1    2050 1100
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR02
U 1 1 68DFEA73
P 2250 1300
F 0 "#PWR02" H 2250 1050 50  0001 C CNN
F 1 "GND" V 2250 1100 50  0000 C CNN
F 2 "" H 2250 1300 50  0001 C CNN
F 3 "" H 2250 1300 50  0001 C CNN
	1    2250 1300
	1    0    0    -1  
$EndComp
Text GLabel 9300 3050 2    50   Input ~ 0
ST1
$Comp
L power:+5V #PWR011
U 1 1 61D92C26
P 3100 2900
F 0 "#PWR011" H 3100 2750 50  0001 C CNN
F 1 "+5V" V 3100 3100 50  0000 C CNN
F 2 "" H 3100 2900 50  0001 C CNN
F 3 "" H 3100 2900 50  0001 C CNN
	1    3100 2900
	0    1    -1   0   
$EndComp
$Comp
L cnc3018_Library:BUCK01 M1
U 1 1 61B26C43
P 2150 3050
F 0 "M1" H 2300 3050 50  0000 C CNN
F 1 "BUCK01" H 2050 3050 50  0000 C CNN
F 2 "0_my_footprints:myMini360BuckConverter" H 2100 3350 50  0001 C CNN
F 3 "" H 2100 3350 50  0001 C CNN
	1    2150 3050
	1    0    0    1   
$EndComp
$Comp
L Diode:1N4001 D1
U 1 1 69362762
P 2650 2900
F 0 "D1" H 2850 2700 50  0000 L CNN
F 1 "1N5819" H 2500 2700 50  0000 L CNN
F 2 "0_my_footprints:myDiodeSchotsky" H 2650 2725 50  0001 C CNN
F 3 "http://www.vishay.com/docs/88503/1n4001.pdf" H 2650 2900 50  0001 C CNN
	1    2650 2900
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR013
U 1 1 693ABDD2
P 2500 3300
F 0 "#PWR013" H 2500 3050 50  0001 C CNN
F 1 "GND" H 2505 3127 50  0000 C CNN
F 2 "" H 2500 3300 50  0001 C CNN
F 3 "" H 2500 3300 50  0001 C CNN
	1    2500 3300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR012
U 1 1 693B1C91
P 1750 3300
F 0 "#PWR012" H 1750 3050 50  0001 C CNN
F 1 "GND" H 1755 3127 50  0000 C CNN
F 2 "" H 1750 3300 50  0001 C CNN
F 3 "" H 1750 3300 50  0001 C CNN
	1    1750 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	2500 3200 2500 3300
Wire Wire Line
	1750 3200 1750 3300
Wire Wire Line
	2800 2900 2950 2900
Connection ~ 2950 2900
Wire Wire Line
	2950 2900 3100 2900
Wire Wire Line
	2950 3200 2500 3200
Connection ~ 2500 3200
$Comp
L power:+12V #PWR01
U 1 1 694CBA8F
P 1950 1300
F 0 "#PWR01" H 1950 1150 50  0001 C CNN
F 1 "+12V" V 1950 1500 50  0000 C CNN
F 2 "" H 1950 1300 50  0001 C CNN
F 3 "" H 1950 1300 50  0001 C CNN
	1    1950 1300
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J4
U 1 1 69C27C1B
P 1750 4200
F 0 "J4" V 2000 4150 50  0000 C CNN
F 1 "12V_TEST" V 1900 4150 50  0000 C CNN
F 2 "0_my_footprints2:pinSocket1x2" H 1750 4200 50  0001 C CNN
F 3 "~" H 1750 4200 50  0001 C CNN
	1    1750 4200
	0    -1   1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J5
U 1 1 69C3DC2B
P 2350 4200
F 0 "J5" V 2600 4150 50  0000 C CNN
F 1 "5V_TEST" V 2500 4150 50  0000 C CNN
F 2 "0_my_footprints2:pinSocket1x2" H 2350 4200 50  0001 C CNN
F 3 "~" H 2350 4200 50  0001 C CNN
	1    2350 4200
	0    -1   1    0   
$EndComp
$Comp
L power:+5V #PWR018
U 1 1 69C4194B
P 2350 4000
F 0 "#PWR018" H 2350 3850 50  0001 C CNN
F 1 "+5V" V 2350 4200 50  0000 C CNN
F 2 "" H 2350 4000 50  0001 C CNN
F 3 "" H 2350 4000 50  0001 C CNN
	1    2350 4000
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR017
U 1 1 69C48BFA
P 1850 4000
F 0 "#PWR017" H 1850 3750 50  0001 C CNN
F 1 "GND" V 1850 3800 50  0000 C CNN
F 2 "" H 1850 4000 50  0001 C CNN
F 3 "" H 1850 4000 50  0001 C CNN
	1    1850 4000
	-1   0    0    1   
$EndComp
$Comp
L power:+12V #PWR016
U 1 1 69C48C00
P 1750 4000
F 0 "#PWR016" H 1750 3850 50  0001 C CNN
F 1 "+12V" V 1750 4200 50  0000 C CNN
F 2 "" H 1750 4000 50  0001 C CNN
F 3 "" H 1750 4000 50  0001 C CNN
	1    1750 4000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR019
U 1 1 69C577D4
P 2450 4000
F 0 "#PWR019" H 2450 3750 50  0001 C CNN
F 1 "GND" V 2450 3800 50  0000 C CNN
F 2 "" H 2450 4000 50  0001 C CNN
F 3 "" H 2450 4000 50  0001 C CNN
	1    2450 4000
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J3
U 1 1 691DDC2F
P 1450 2900
F 0 "J3" H 1750 2900 50  0000 C CNN
F 1 "BUCK_PWR" H 1750 2800 50  0000 C CNN
F 2 "0_my_footprints2:pinHeader1x2" H 1450 2900 50  0001 C CNN
F 3 "~" H 1450 2900 50  0001 C CNN
	1    1450 2900
	-1   0    0    1   
$EndComp
$Comp
L power:+12V #PWR010
U 1 1 691E044F
P 1650 2700
F 0 "#PWR010" H 1650 2550 50  0001 C CNN
F 1 "+12V" H 1650 2900 50  0000 C CNN
F 2 "" H 1650 2700 50  0001 C CNN
F 3 "" H 1650 2700 50  0001 C CNN
	1    1650 2700
	1    0    0    -1  
$EndComp
Wire Wire Line
	1650 2700 1650 2800
Wire Wire Line
	1650 2900 1750 2900
$Comp
L Connector_Generic:Conn_01x03 J2
U 1 1 691B72AF
P 3000 1100
F 0 "J2" V 3150 800 50  0000 L CNN
F 1 "SEATALK" V 3150 1000 50  0000 L CNN
F 2 "0_my_footprints2:JST3" H 3000 1100 50  0001 C CNN
F 3 "~" H 3000 1100 50  0001 C CNN
	1    3000 1100
	0    1    -1   0   
$EndComp
$Comp
L power:+12V #PWR03
U 1 1 691B842B
P 2900 1300
F 0 "#PWR03" H 2900 1150 50  0001 C CNN
F 1 "+12V" V 2900 1400 50  0000 L CNN
F 2 "" H 2900 1300 50  0001 C CNN
F 3 "" H 2900 1300 50  0001 C CNN
	1    2900 1300
	-1   0    0    1   
$EndComp
Text GLabel 3000 1300 3    50   Input ~ 0
ST1
$Comp
L power:GND #PWR04
U 1 1 691BA5E9
P 3100 1300
F 0 "#PWR04" H 3100 1050 50  0001 C CNN
F 1 "GND" V 3100 1100 50  0000 C CNN
F 2 "" H 3100 1300 50  0001 C CNN
F 3 "" H 3100 1300 50  0001 C CNN
	1    3100 1300
	1    0    0    -1  
$EndComp
Wire Wire Line
	8900 3250 8900 3750
Wire Wire Line
	8650 3750 8900 3750
Connection ~ 8900 3750
Wire Wire Line
	8900 3750 8900 3850
Wire Wire Line
	9100 3050 9200 3050
Wire Wire Line
	9200 3050 9200 3550
Wire Wire Line
	9200 3550 8650 3550
Connection ~ 9200 3050
Wire Wire Line
	9200 3050 9300 3050
$Comp
L Device:R R1
U 1 1 6822159F
P 7900 2550
F 0 "R1" H 7970 2596 50  0001 L CNN
F 1 "10K" V 7900 2550 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 7830 2550 50  0001 C CNN
F 3 "~" H 7900 2550 50  0001 C CNN
	1    7900 2550
	1    0    0    -1  
$EndComp
Text GLabel 5100 2300 1    50   Input ~ 0
CRX
Text GLabel 5000 2300 1    50   Input ~ 0
CTX
$Comp
L power:GND #PWR020
U 1 1 69A1A92C
P 4700 4400
F 0 "#PWR020" H 4700 4150 50  0001 C CNN
F 1 "GND" V 4700 4200 50  0000 C CNN
F 2 "" H 4700 4400 50  0001 C CNN
F 3 "" H 4700 4400 50  0001 C CNN
	1    4700 4400
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR05
U 1 1 68C5F036
P 4700 2300
F 0 "#PWR05" H 4700 2150 50  0001 C CNN
F 1 "+5V" V 4700 2400 50  0000 L CNN
F 2 "" H 4700 2300 50  0001 C CNN
F 3 "" H 4700 2300 50  0001 C CNN
	1    4700 2300
	1    0    0    -1  
$EndComp
Text GLabel 5900 2300 1    63   Input ~ 0
TX3
$Comp
L power:+3V3 #PWR07
U 1 1 681F6BDE
P 4900 2300
F 0 "#PWR07" H 4900 2150 50  0001 C CNN
F 1 "+3V3" V 4900 2500 50  0000 C CNN
F 2 "" H 4900 2300 50  0001 C CNN
F 3 "" H 4900 2300 50  0001 C CNN
	1    4900 2300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR06
U 1 1 681F5B40
P 4800 2300
F 0 "#PWR06" H 4800 2050 50  0001 C CNN
F 1 "GND" V 4800 2100 50  0000 C CNN
F 2 "" H 4800 2300 50  0001 C CNN
F 3 "" H 4800 2300 50  0001 C CNN
	1    4800 2300
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x04 J7
U 1 1 69A3F90A
P 2600 5600
F 0 "J7" V 2750 5800 50  0000 R CNN
F 1 "NEO6M" V 2750 5600 50  0000 R CNN
F 2 "0_my_footprints2:neo6m" H 2600 5600 50  0001 C CNN
F 3 "~" H 2600 5600 50  0001 C CNN
	1    2600 5600
	0    -1   -1   0   
$EndComp
Text GLabel 2700 5800 3    63   Input ~ 0
TX3
Text GLabel 2600 5800 3    63   Input ~ 0
RX3
Text Notes 2450 5550 0    50   ~ 0
G TX RX V
$Comp
L power:GND #PWR023
U 1 1 69A4C897
P 2500 5800
F 0 "#PWR023" H 2500 5550 50  0001 C CNN
F 1 "GND" V 2500 5600 50  0000 C CNN
F 2 "" H 2500 5800 50  0001 C CNN
F 3 "" H 2500 5800 50  0001 C CNN
	1    2500 5800
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR024
U 1 1 69A4C89D
P 2800 5800
F 0 "#PWR024" H 2800 5650 50  0001 C CNN
F 1 "+3V3" V 2800 6000 50  0000 C CNN
F 2 "" H 2800 5800 50  0001 C CNN
F 3 "" H 2800 5800 50  0001 C CNN
	1    2800 5800
	-1   0    0    1   
$EndComp
Text GLabel 5800 2300 1    63   Input ~ 0
RX3
Text GLabel 7550 3750 0    63   Input ~ 0
TX4
Text GLabel 7550 2850 0    63   Input ~ 0
RX4
Text GLabel 5700 2300 1    63   Input ~ 0
RX4
Text GLabel 5600 2300 1    63   Input ~ 0
TX4
$Comp
L 0_my_teensy:myTeensy4.0 U2
U 1 1 681E4C83
P 5250 3300
F 0 "U2" H 6078 3303 60  0001 L CNN
F 1 "myTeensy4.0" H 5100 3200 60  0000 L CNN
F 2 "0_my_teensy:teensy40" V 6150 3350 60  0001 C CNN
F 3 "" V 6150 3350 60  0000 C CNN
	1    5250 3300
	1    0    0    -1  
$EndComp
$EndSCHEMATC
