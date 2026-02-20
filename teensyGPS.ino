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
// 13 - ALIVE_LED
// 14 - TX3 Neo6m
// 15 - RX3 Neo6m
// 16 - RX4 Seatalk
// 17 - TX4 Seatalk
// 19 - Seatalk Enabled
// 20 - NMEA2000 Enabled
// 22 - CTX to CANBUS module
// 23 - CRX from CANBUS module

#include <myDebug.h>
#include "neoGPS.h"
#include <instST.h>
#include <inst2000.h>
#include <instSimulator.h>
#include <boatSimulator.h>

#define ALIVE_LED		13
#define ALIVE_OFF_TIME	980
#define ALIVE_ON_TIME	20


// NMEA2000

static const unsigned long gpsTransmitMessages[] = {
	// these system PGNs may not be necessary here,
	// but it is more conformal to include them.
#if 1
	PGN_REQUEST,
	PGN_ADDRESS_CLAIM,
	PGN_PGN_LIST,
	PGN_HEARTBEAT,
	PGN_PRODUCT_INFO,
	PGN_DEVICE_CONFIG,
#endif

	PGN_SYSTEM_DATE_TIME,
	// PGN_VESSEL_HEADING,
	// PGN_HEADING_TRACK_CONTROL,
	// PGN_ENGINE_RAPID,
	// PGN_ENGINE_DYNAMIC,
	// PGN_AC_INPUT_STATUS,
    // PGN_AC_OUTPUT_STATUS,
	// PGN_FLUID_LEVEL,
	// PGN_AGS_CONFIG_STATUS,
	// //PGN_AGS_STATUS,
	// PGN_SPEED_WATER_REF,
	// PGN_WATER_DEPTH,
	// PGN_DISTANCE_LOG,
	// //PGN_POSITION_RAPID_UPDATE,
	// //PGN_COG_SOG_RAPID_UPDATE,
	PGN_GNSS_POSITION_DATA,
	// //PGN_LOCAL_TIME_OFFSET,
	// //PGN_AIS_CLASS_B_POSITION,
	// //PGN_DATUM,
	// PGN_CROSS_TRACK_ERROR,
	// PGN_NAVIGATION_DATA,
	// //PGN_ROUTE_WP_INFO,
	// //PGN_SET_AND_DRIFT,
	PGN_GNSS_SATS_IN_VIEW,
	// //PGN_AIS_STATIC_B_PART_A,
	// //PGN_AIS_STATIC_B_PART_B.
	// PGN_WIND_DATA,
	// //PGN_ENV_PARAMETERS,
	// //PGN_TEMPERATURE,
	PGN_DIRECTION_DATA,
	// PGN_SEATALK_ROUTE_INFO,
    //
	// PGN_SEATALK_ROUTE_INFO,
    //
	// PGN_GEN_PHASE_A_AC_POWER,
	// PGN_GEN_PHASE_A_BASIC_AC,
	// PGN_TOTAL_AC_POWER,
	// PGN_AVERAGE_AC_QUANTITIES,
	// PGN_SEATALK_GEN_INFO,

	0
};



//---------------------------------
// Help
//---------------------------------

static void showHelp()
{
	display(0,"",0);
	display(0,"teensyGPS Help SEATALK_ENABLED(%d) NMEA2000_ENABLED(%d)",seatalk_enabled,nmea2000_enabled);
	display(0,"",0);
	proc_entry();
	display(0,"?       = show help",0);
	display(0,"reboot  = Reboot the teensyGPS",0);
	display(0,"L       = Show NMEA2000 Device List",0);
	display(0,"Q       = Query NMEA2000 Devices",0);
	display(0,"",0);
	display(0,"",0);
	display(d,"Monitoring",0);
	display(d,"",0);
	display(0,"M_ST   = N    monitor Seatalk messags",0);
	display(0,"M_2000 = N    monitor known NMEA2000 sensor messages",0);
	display(0,"              0x0001	= sensors out, known messages in",0);
	display(0,"              0x0002 = GPS/AIS specifically",0);
	display(0,"              0x0004 = known proprietary in",0);
	display(0,"              0x0008 = unknown (not busi.e. proprietary) in",0);
	display(0,"              0x0010 = BUS in",0);
	display(0,"              0x0020 = BUS out",0);
	display(0,"              0x1000	= self (sent) as well as received",0);
	display(0,"              0x8000	= show raw 'instrument' messages",0);
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
	
	initNeoGPS();

	if (seatalk_enabled)
	{
		display(0,"Opening SERIAL_ST",0);
		ST_SERIAL.begin(4800, SERIAL_9N1);
	}

	if (nmea2000_enabled)
	{
		display(0,"Initializing NMEA2000",0);
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

		nmea2000.ExtendTransmitMessages(gpsTransmitMessages);
			// different list of transmit messages
		nmea2000.init(TEENSYGPS_NMEA_ADDRESS);
	}


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
		showHelp();
	else if (lval.equals("l"))
		nmea2000.listDevices();
	else if (lval.equals("q"))
		nmea2000.sendDeviceQuery();
	else if (lval == "reboot")
	{
		warning(0,"REBOOTING teensyGPS!!",0);
		delay(300);
		SCB_AIRCR = 0x05FA0004;
		while (1) { delay(1000); }
	}
	
	// monitor (subzet of teensyboat)

	else if (lval.startsWith("m_"))
	{
		String what = lval.substring(2);
		uint32_t value = hexOrUint(rval);
		display(0,"monitor %s=0x%08x",what.c_str(),value);

		int port = -1;
		if (what.equals("st"))	port = PORT_ST1;
		else if (what.equals("2000"))	port = PORT_2000;
		else
			my_error("invalid monitor command(%s)=%d",what.c_str(),value);

		if (port != -1)
			inst_sim.setMonitor(port,value);
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






//--------------------------------------------
// loop()
//--------------------------------------------

void loop()
{
	doNeoGPS();

	//-------------------
	// SEATALK
	//-------------------

	if (seatalk_enabled)
		handleStPort();

	//---------------------
	// NMEA200
	//----------------------

	if (nmea2000_enabled)
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
