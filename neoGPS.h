//----------------------------------------------
// neoGPS.h
//----------------------------------------------
// replace much of complexity of Boat library
// by not including instSimulator.h or boatSimulator.h

#pragma once

#define ST_SERIAL				Serial4
#define NEO_SERIAL				Serial3


#define MAX_PRN 				32

#define SAT_IN_VIEW     		0x01   // appeared in GSV this cycle
#define SAT_USED_IN_SOLUTION    0x02   // listed in GSA (used in solution)

typedef struct
{
    int  elev;
    int  azim;
    int  snr;
    uint32_t flags;
} gps_sat_t;


typedef struct
{
    int    	fix_type;			// NMEA0183 Fix type
    double 	lat;
    double 	lon;
    double 	altitude;
    float  	hdop;
	float  	vdop;
    float  	pdop;
    float 	sog;      			// knots
    float 	cog;      			// degrees
	int    	num_viewed;			// as per GSV
    int    	num_used;			// in solution as per GSA
    gps_sat_t sats[MAX_PRN];
    int    	year;
    int    	month;
    int    	day;
    int    	hour;
    int    	minute;
    int  	seconds;

} gps_model_t;


// in teensyGPS.ino

extern uint8_t seatalk_enabled;
extern uint8_t nmea2000_enabled;

// in neoGPS.cpp

extern void initNeoGPS();
extern void doNeoGPS();
extern gps_model_t gps_model;

// in neoST.cpp

extern void handleStPort();
extern void sendNeoST();

// in neo2000.cpp

extern void sendNeo2000();






