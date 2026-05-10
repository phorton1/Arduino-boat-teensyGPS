# teensyGPS

**Home**

repos: **[phorton1](https://github.com/phorton1)** --
**[teensyBoat Firmware](https://github.com/phorton1/Arduino-boat-teensyBoat/blob/master/docs/readme.md)** --
**[teensyBoat App](https://github.com/phorton1/base-apps-teensyBoat/blob/master/docs/readme.md)** --
**[Boat Library](https://github.com/phorton1/Arduino-libraries-Boat/blob/master/docs/readme.md)** --
**[tbESP32 WiFi](https://github.com/phorton1/Arduino-boat-tbESP32/blob/master/docs/readme.md)** --
**[teensyWind Tester](https://github.com/phorton1/Arduino-boat-teensyWind/blob/master/docs/readme.md)** --
**teensyGPS**

**teensyGPS** is a Teensy 4.0 firmware and custom PCB that turns a u-blox
**Neo-6M** GPS module into a drop-in GPS source for Raymarine marine instrument
networks.  It reads the standard NMEA 0183 sentence stream from the Neo-6M,
builds a complete GPS model (position, COG, SOG, fix quality, satellite data,
date and time), syncs the Teensy's real-time clock from the GPS fix, and
re-broadcasts everything as **Seatalk1** datagrams and/or **NMEA 2000** packets
every second.

The device is built on the shared
**[Arduino Boat Library](https://github.com/phorton1/Arduino-libraries-Boat/blob/master/docs/readme.md)**,
which provides the Seatalk1 encoding helpers and the NMEA 2000 object.  It
lives in a 3D-printed enclosure mounted on a custom home-milled PCB.

TODO: insert photo of teensyGPS device here


## Protocol Output

### Seatalk1

When Seatalk1 output is enabled, teensyGPS transmits a complete set of GPS
datagrams every second:

- **Position** (ST_LATLON, 0x58) -- latitude and longitude
- **COG** (0x53) -- course over ground, converted to magnetic using the
  Boat library's magnetic declination model
- **SOG** (0x52) -- speed over ground in knots
- **Date** (0x56) and **Time** (0x54) -- UTC date and time from GPS fix
- **Satellite summary** (ST_SAT_INFO, 0x57) -- fix type and HDOP
- **Satellite detail** (ST_SAT_DETAIL, 0xA5) -- fix quality, HDOP fields
- **Per-satellite data** -- PRN, elevation, azimuth, SNR, and used-in-solution
  flag, up to the Seatalk maximum, in PRN order

The device also **receives** and responds to Seatalk1:

- **Device queries** (0xA4) -- identifies itself as a Raymarine RS125 GPS
  (Seatalk unit ID 0xC5)
- **Restart GPS button** on the Raymarine E80 -- a specific 0xA5/0x4D
  datagram pattern causes the firmware to send the expected reply and then
  reboot, making the E80's Restart GPS button work correctly

### NMEA 2000

When NMEA 2000 output is enabled, teensyGPS transmits four PGNs per second:

| PGN    | Name                    | Content                                      |
| ------ | ----------------------- | -------------------------------------------- |
| 129029 | GNSS Position Data      | lat, lon, altitude, fix type, sats, DOP      |
| 130577 | Direction Data          | COG (true) and SOG                           |
| 126992 | System Date/Time        | UTC time from GPS, GPS source                |
| 129540 | GNSS Satellites in View | per-satellite PRN, elev, azimuth, SNR, usage |

The device registers with NMEA 2000 as a **Positioning System** (function 130,
class 60) named "teensyGPS_device".


## Serial Console and Configuration

teensyGPS has a persistent **serial console UI** accessible via any USB serial
terminal at 921600 baud.  Settings are stored in EEPROM (base offset 256,
separate from the teensyBoat range at 512) and survive power cycles.

Type `?` for help.  Commands use the `NAME=VALUE` or bare-word format:

| Command        | Description                                          |
| -------------- | ---------------------------------------------------- |
| `SEATALK=0/1`  | Enable or disable Seatalk1 output (default: enabled) |
| `NMEA2000=0/1` | Enable or disable NMEA 2000 output (default: off)    |
| `SAVE`         | Save current settings to EEPROM                      |
| `LOAD`         | Load settings from EEPROM                            |
| `reboot`       | Software-reboot the Teensy                           |
| `L`            | List all NMEA 2000 devices on the bus (with product info) |
| `Q`            | Send a device query to all NMEA 2000 devices         |
| `M_ST=N`       | Monitor incoming Seatalk1 datagrams (0=off, 1=on)    |
| `M_2000=N`     | Monitor NMEA 2000 traffic (bitmask, see below)       |

### Protocol Monitoring

The monitoring commands tap into the Boat library's full **named-datagram
decoders**.  With `M_ST=1`, every incoming Seatalk1 datagram is decoded and
printed by name -- DEPTH, WIND_ANGLE, WIND_SPEED, WATER_SPEED, SOG, COG,
LATLON, SAT_INFO, AUTOPILOT, and others -- making teensyGPS a useful
live Seatalk1 bus monitor as well as a GPS source.

With `M_2000`, NMEA 2000 traffic is decoded and printed with PGN name,
priority, source address, and data, with separate bitmask control over
which traffic categories are shown:

| Bit    | Meaning                                      |
| ------ | -------------------------------------------- |
| 0x0001 | Sensor output and known incoming messages    |
| 0x0002 | GPS and AIS messages specifically            |
| 0x0004 | Known proprietary incoming messages          |
| 0x0008 | Unknown (non-bus, non-proprietary) incoming  |
| 0x0010 | Bus traffic in                               |
| 0x0020 | Bus traffic out                              |
| 0x1000 | Include self-sent messages in monitoring     |
| 0x8000 | Show raw instrument messages                 |

The `L` and `Q` commands use the Boat library's NMEA 2000 device list to
enumerate all devices on the bus and display their manufacturer name,
model ID, software version, and NMEA 2000 source address -- useful for
understanding what else is on the network.


## Genuine vs. Clone Neo-6M

At startup teensyGPS sends a UBX-MON-VER query to the GPS module and checks the
response for the string "SW VERSION:" -- a signature present in genuine u-blox
modules but absent in common clone modules.  Genuine modules do not need to be
configured to suppress GLONASS and BeiDou sentences; clones cannot be configured
to suppress them.  Either type works: teensyGPS filters the constellation
explicitly, processing only `$GPGSV` sentences (GPS satellites) and ignoring
`$BDGSV` (BeiDou) to maintain E80 chartplotter compatibility.


## Boat Library Dependency

teensyGPS uses the
**[Arduino Boat Library](https://github.com/phorton1/Arduino-libraries-Boat/blob/master/docs/readme.md)**
for three things:

- **`instST.h`** -- Seatalk1 datagram queue, send, and satellite-data helper
  functions (`queueDatagram`, `sendDatagram`, `initStSatMessages`,
  `addStSatMessage`, `queueStSatMessages`, `clearSTQueues`, and the
  `ST_*` datagram constants)
- **`inst2000.h`** -- the NMEA 2000 bus object and PGN send helpers
- **`boatSimulator.h`** -- `boat_sim.makeMagnetic()` to convert true COG
  to magnetic before encoding the Seatalk1 COG datagram


## Files Available

Hardware design files are included in this repository:

- **docs/kicad/gps_board/** -- KiCad schematic and PCB layout for the GPS board.
  TODO: insert schematic image here
  TODO: insert PCB layout image here
- **docs/kicad/gps_board/plot/** -- Gerber files for fabrication.
- **docs/kicad/gps_board/plot/gcode/** -- CNC milling gcode for the home-milled
  PCB: isolate, laser mark, drill, mill holes, cutout.
- **docs/3dp/** -- 3D-printed enclosure parts:
  - **gps_case.stl** -- main enclosure body
  - **gps_top.stl** -- top cover
  - **gps_mount.stl** -- mounting bracket (printed in ASA for UV resistance)
  - **neo_support.stl** -- internal support bracket for the Neo-6M module
  - **gps_test_nut1.stl** and **gps_test_nut2.stl** -- connector test fixtures
  - **gps_box.f3d** -- Fusion 360 source file for the enclosure


## Credits

- [**Paul Stoffregen**](https://www.pjrc.com/teensy/) --
  Creator of the Teensy microcontroller platform.  teensyGPS is built
  for the Teensy 4.0.

- [**Thomas Knauf**](http://www.thomasknauf.de/seatalk.htm) --
  *SeaTalk Technical Reference Revision 3.23*.  The primary public reference for
  the Seatalk1 datagram protocol.  The
  **[Arduino Boat Library](https://github.com/phorton1/Arduino-libraries-Boat/blob/master/docs/readme.md)**
  used by this firmware builds on, extends, and in places corrects that work.


## License

This software is released under the
[**GNU General Public License v3**](../LICENSE.TXT).


## Please Also See

- [**phorton1/Arduino-libraries-Boat**](https://github.com/phorton1/Arduino-libraries-Boat) --
  The shared Arduino C++ library used by this firmware.  Provides Seatalk1 and
  NMEA 2000 protocol encoding, satellite data helpers, and the magnetic
  declination model.

- [**phorton1/Arduino-boat-teensyBoat**](https://github.com/phorton1/Arduino-boat-teensyBoat) --
  The related multi-protocol marine instrument bridge using the same Boat library.
  Bridges Seatalk1, NMEA 0183, and NMEA 2000; includes a virtual boat simulator
  and a desktop test harness for Raymarine ST50 instruments.
