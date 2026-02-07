//-------------------------------------------
// teensyGPS.ino
//-------------------------------------------
// A neo6m based GPS device that uses the Boat library
// for encoding and transport.
//
// Contains a nominal USB Serial UI that can turn the GPS unit (parser)
// on and off and display debugging information.
//
// Connects to the neo6m device via the NEO_SERIAL port (default = Serial5)
// and parses nmea0183 messags into a model that can be sent out via
// Seatalk (and/or NMEA2000).


//------------------------
// Teensy Pins Used
//------------------------
// 23 - CRX from CANBUS module
// 22 - CTX to CANBUS module
// 0  - RX1 Seatalk1
// 1  = TX1 Seatalk1
// 15 - RX3 Neo6m
// 14 - TX3 Neo6m
// 9  - ALIVE_LED

#include <myDebug.h>
#include "neoGPS.h"
#include <EEPROM.h>

#define dbg_eeprom 		0


#define ALIVE_LED		9
#define ALIVE_OFF_TIME	980
#define ALIVE_ON_TIME	20

#define SERIAL_ST1		Serial1
#define NEO6M_SERIAL	Serial3

// Seatalk

#include <instST.h>
#define ST_IDLE_BUS_MS				10		// ms bus must be idle to send next datagram
#define ST_SEND_INTERVAL			10
#define USE_E80_PORT2				0		// "port2" param for calls to Boat instST_out routines of SERIAL_ST1

// NMEA2000

#include <inst2000.h>



//-----------------------------------
// EEPROM
//-----------------------------------
// Separate from teensyBoat range which starts at 512

#define GPS_EEPROM_BASE 	256


static void saveToEEPROM()
{
	int offset = GPS_EEPROM_BASE;
	EEPROM.write(offset++,NeoSeatalkEnabled());
	EEPROM.write(offset++,NeoNMEA2000Enabled());
	display(dbg_eeprom,"EEPROM saved SEATALK=%d NMEA2000=%d",
		NeoSeatalkEnabled(),
		NeoNMEA2000Enabled());
}


void loadFromEEPROM()
{
	int offset = GPS_EEPROM_BASE;
	uint8_t enable_seatalk = EEPROM.read(offset++);
	if (enable_seatalk == 255) enable_seatalk = 0;
	uint8_t enable_nmea2000 = EEPROM.read(offset++);
	if (enable_nmea2000 == 255) enable_nmea2000 = 0;
	display(dbg_eeprom,"EEPROM loaded SEATALK=%d NMEA2000=%d",
		enable_seatalk,
		enable_nmea2000);
	enableNeoSeatalk(enable_seatalk);
	enableNeoNMEA200(enable_nmea2000);
}



//---------------------------------
// Help
//---------------------------------

static void showHelp(bool detailed)
{
	// int d = detailed ? 0 : 1;

	display(0,"",0);
	display(0,"teensyBoat Help",0);
	proc_entry();
	display(0,"?              show condensed help",0);
	display(0,"help           show detailed help",0);

	display(0,"",0);
	display(0,"SEATALK  = N    Send out via Seatalk; cur=%d",NeoSeatalkEnabled());
	display(0,"NMEA2000 = N    Send out via Seatalk; default=%d",NeoNMEA2000Enabled());

	display(0,"",0);
	display(0,"Save to EEPROM",0);
	display(0,"    LOAD = load the current instrument configuration to EEPROM",0);
	display(0,"    SAVE = save the current instrument configuration to EEPROM",0);

	display(0,"",0);
	display(0,"Monadic Commands",0);
	display(0,"    REBOOT = Reboot the teensyGPS",0);
	// display(0,"    STATE  = return the state of the satellites",0);
	display(0,"    L      = monadic command to Show NMEA2000 Device List",0);
	display(0,"    Q      = monadic command to Query NMEA2000 Devices",0);
	display(0,"",0);
	
	proc_leave();
}


//------------------------
// setup
//------------------------

void setup()
{
	#if ALIVE_LED
		pinMode(ALIVE_LED,OUTPUT);
		digitalWrite(ALIVE_LED,1);
	#endif

	Serial.begin(921600);	// !!! 115200
	delay(2000);
	display(0,"teensyGPS.ino setup() started",0);
	proc_entry();
	
	loadFromEEPROM();

	initNeo6M_GPS(&NEO6M_SERIAL,USE_E80_PORT2,1,0x99);

	SERIAL_ST1.begin(4800, SERIAL_9N1);

	// nmea2000 initialization

	nmea2000.SetProductInformation(
		"teensyGPS",            		// Manufacturer's Model serial code
		2000,                        	// Manufacturer's uint8_t product code
		"teensyGPS_device",   			// Manufacturer's Model ID
		"tg_sw_1.0",             		// Manufacturer's Software version code
		"tg_v_1.0",             		// Manufacturer's uint8_t Model version
		3,                          	// LoadEquivalency uint8_t 3=150ma; Default=1. x * 50 mA
		2101,                       	// N2kVersion Default=2101
		1,                          	// CertificationLevel Default=1
		0                           	// iDev (int) index of the device on \ref Devices
		);
	nmea2000.SetConfigurationInformation(
		"prhSystems",      // ManufacturerInformation
		"tbInstall1",      // InstallationDescription1
		"tbInstall2"       // InstallationDescription2
		);
	nmea2000.SetDeviceInformation(
		234567,  // uint32_t Unique number, i.e. Serial number.
		130,     // uint8_t  Device function = Positioning System
		60,      // uint8_t  Device class = External Environment
		2046     // uint16_t Registration/Company) ID // 2046 does not exist; choosen arbitrarily
		);
	nmea2000.init(TEENSYGPS_NMEA_ADDRESS);


	proc_leave();
	display(0,"teensyGPS.ino  setup() finished",0);
	display(0,"type ?<enter> for help",0);
}


//--------------------------------------------------
// handleCommand()
//--------------------------------------------------

uint16_t hexOrUint(const String &str)
	// // base 0 auto-detects "0x" prefix
{
	return strtol(str.c_str(), nullptr, 0);
}


static void handleCommand(String lval, String rval, bool got_equals)
{
	display(0,"command: %s%s%s",lval.c_str(),got_equals?"=":"",rval.c_str());

	if (lval == "?")
		showHelp(1);
	else if (lval.equals("l"))
		nmea2000.listDevices();
	else if (lval.equals("q"))
		nmea2000.sendDeviceQuery();

	// Warning: changing modes, particularly to Seatalk may
	// require a reboot of the E80.

	else if (lval == "seatalk")
	{
		bool enable = rval.toInt();
		// if (enable)
		// 	enableNeoNMEA200(false);
		enableNeoSeatalk(enable);
	}
	else if (lval == "nmea2000")
	{
		bool enable = rval.toInt();
		// if (enable)
		// 	enableNeoSeatalk(false);
		enableNeoNMEA200(enable);
	}

	// EEPROM

	else if (lval == "save")
		saveToEEPROM();
	else if (lval == "load")
		loadFromEEPROM();

	// other

	else if (lval == "reboot")
	{
		warning(0,"REBOOTING teensyGPS!!",0);
		delay(300);
		SCB_AIRCR = 0x05FA0004;
		while (1) { delay(1000); }
	}
	
	// unknown command

	else
		my_error("unknown command: %s%s%s",lval.c_str(),got_equals?"=":"",rval.c_str());

}	// handleCommand()



//------------------------------------------------
// handleSerial()
//------------------------------------------------

static void handleSerial()
{
	// Serial UI

	if (Serial.available())
	{
		static String lval;
		static String rval;
		static bool got_equals;
		char c = Serial.read();

		if (c == 0x0a)
		{
			handleCommand(lval.toLowerCase(),rval.toLowerCase(),got_equals);
			lval = "";
			rval = "";
			got_equals = 0;
		}
		else if (c == '=')
		{
			got_equals = 1;
		}
		else if (c != 0x0d)
		{
			if (got_equals)
				rval += c;
			else
				lval += c;
		}
	}
}	// handleSerial()



//-------------------------------------------------------------
// Minimal ST parser for DEV_QUERY and Restart GPS Button
//------------------------------------------------------------
// Restart GPS button

#define dbg_parse 0

static void parseDatagram(uint8_t *dg)
{
	if (dg[0] == 0xa4 &&	// ST_DEV_QUERY == 0x1a4
	   (dg[1] == 0x06 || dg[1] == 0x02))
	{
		warning(dbg_parse,"DEV_QUERY",0);
		st_neo_device_query_pending = 1;
	}
	else if (
		dg[0] == 0xa5 && 	// ST_SAT_DETAIL = 0x1a5
		dg[1] == 0x4d &&
		dg[15] == 0x08)
	{
		warning(dbg_parse,"RESTART GPS",0);
		replyToRestartGPSButton();
	}
}




static void handleStPort()
{

	static uint32_t last_st_read;
	static uint32_t last_st_write;
	static int outp = 0;
	static int dlen = 0;
	static uint8_t datagram[MAX_ST_BUF];

	// purge the input buffer

	while (SERIAL_ST1.available())
	{
		int c = SERIAL_ST1.read();
		last_st_read = millis();

		// the 9th bit is set on the first 'byte' of a sequence
		// the low nibble of the 2nd byte + 3 is the total number
		// 		of bytes, so all messages are at least 3 bytes
		//		the high nibble may be data.
		//	data[n+3];, implying a maximum datagram size of 19
		//  this routine can never receive more than 19 bytes
		//	but MAX_ST_BUF is set to 20 for neatness

		#if 0
			display(0,"ST got 0x%02x '%c'",c,(c>32 && c<128)?c:' ');
		#endif

		if (c > 0xff)
		{
			if (outp)
			{
				my_error("Dropped datagram ",0);
				outp = 0;
			}
			datagram[outp++] = c;
		}
		else if (outp == 1)
		{
			dlen = (c & 0x0f) + 3;
			datagram[outp++] = c;
		}
		else if (outp < dlen)
		{
			datagram[outp++] = c;
			if (outp == dlen)
			{
				parseDatagram(datagram);
				outp = 0;
				dlen = 0;
			}
		}
		else
		{
			my_error("unexpected byte 0x%02x '%c'",c,(c>32 && c<128)?c:' ');
		}

	}	// receiving datagrams


	// send one datagram from the ST port's queue

	uint32_t now_st = millis();
	if (now_st - last_st_read >= ST_IDLE_BUS_MS &&
		now_st - last_st_write > ST_SEND_INTERVAL)
	{
		sendDatagram(USE_E80_PORT2);
		last_st_write = millis();
	}
}




//--------------------------------------------
// loop()
//--------------------------------------------

void loop()
{
	doNeo6M_GPS();

	//-------------------
	// SEATALK
	//-------------------

	handleStPort();

	//---------------------
	// NMEA200
	//----------------------

	// if (NeoNMEA2000Enabled())
	{
		nmea2000.ParseMessages(); // Keep parsing messages
		#if BROADCAST_NMEA2000_INFO
			nmea2000.broadcastNMEA2000Info();
		#endif
	}
	
	//-------------------
	// UI
	//-------------------

	#if ALIVE_LED
		static bool alive_on = 0;
		static uint32_t last_alive_time = 0;
		uint32_t alive_now = millis();
		uint32_t alive_delay = alive_on ? ALIVE_ON_TIME : ALIVE_OFF_TIME;
		if (alive_now - last_alive_time >= alive_delay)
		{
			alive_on = !alive_on;
			digitalWrite(ALIVE_LED,alive_on);
			last_alive_time = alive_now;
		}
	#endif
	
	// USB SERIAL

	handleSerial();

}	// loop()


// end of teensyGPS.ino
