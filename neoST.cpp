//----------------------------------------
// neoST.cpp
//----------------------------------------

#include <myDebug.h>
#include "neoGPS.h"
#include <instST.h>
#include <boatSimulator.h>

#define dbg_st_out		1
#define dbg_st_in 		0

#define MAX_ST_BUF					20		// 20 byte max to st messages

#define ST_IDLE_BUS_MS				10		// ms bus must be idle to send next datagram
#define ST_SEND_INTERVAL			10



static volatile bool st_neo_device_query_pending;
	// set by client code (instST_in.cpp or teensyGPS.ino) when a device query is
	// received, in which case, the neo will reply with a device id message


// extern
void sendNeoST()
	// implementation copied from instST_out.cpp::gpsInst:;sendSeatalk()
	// and just fills up to ST limits in PRN order
{
	display(dbg_st_out,"sendNeoST()",0);

	// handle device requires separate from instSimulator
	// we return version 1.99 to differentiate from instST_out which returns version 1.01

	uint16_t dg[MAX_ST_BUF];
	if (st_neo_device_query_pending)
	{
		#define ST_DEVICE_ID		0xc5	// Unit ID = RS125 GPS
		#define ST_MAJOR_VERSION	1
		#define ST_MINOR_VERSION	0x99

		warning(0,"neoGPS sending DEV_QUERY(%02x) v%d.%x",
			ST_DEVICE_ID,
			ST_MAJOR_VERSION,
			ST_MINOR_VERSION);

		dg[0] = ST_DEV_QUERY;			// 0x1a4
		dg[1] = 0x12;      				// 0x10 constant + length
		dg[2] = ST_DEVICE_ID;      		// Unit ID
		dg[3] = ST_MAJOR_VERSION;      	// Main SW version
		dg[4] = ST_MINOR_VERSION;     	// Minor SW version
		queueDatagram(0,dg);

		st_neo_device_query_pending = 0;
		return;
	}

	proc_entry();


	//-----------------------
	// ST_SAT_INFO
	//-----------------------
	// Use arbitray value of 0/5 satelites based on fix

	if (1)
	{
		// NOTE ST_SAT_INFO does not like numsats=11 (0xb0)
		// *perhaps* it is actually the number of sats used in the solution
		// and, thus would be limited to 9

		display(dbg_st_out+1,"neoGPS sending st_sat_info",0);
		dg[0] = ST_SAT_INFO;	// 0x157
		dg[1] = gps_model.fix_type ? 0x50 : 0x00;      		// num_sats USED=5 << 4
		dg[2] = gps_model.fix_type ? 0x02 : 0xff;          	// HDOP = 2
		queueDatagram(0,dg);
	}

	//------------------------------------------------
	// SAT_DET_INFO (A5 57)  — GPS Fix + HDOP block
	//------------------------------------------------
	// note we send hdop 3 from this as opposed to 2 above

	if (1)
	{
		display(dbg_st_out+1,"neoGPS sending st_sat_detail info",0);

		dg[0] = ST_SAT_DETAIL;		// // 0x1A5
		dg[1] = 0x57;

		// QQ
		// 		fix = QQ&0xF 						    = .... 00nn = 0x0nn = 1 "Fix"
		//		fix_available = QQ&0x10 			    = ...1 .... = 0x10 = 1
		// 		high 3 bits of numsats=5= QQ&0xE0/16  	= 010. .... = 0x40 = 2
		// Knauf uses QQ&0xE0/16 to mean (QQ&0xE0)>>4 in exprssion for numsats

		uint8_t QQ = 0;
		if (gps_model.fix_type > 0)
			QQ = (gps_model.fix_type & 0x0f) | 0x10 | 0x40;
		dg[2] = QQ;

		// HH:
		//		HDOP = HH&0x7C = HH & 01111100			= .000 11.. = 0x0C = 3
		//		HDOP availability = HH&0x80 		    = 1... .... = 0x80 = 1
		//		low bit of numsats = HH&0x01			= .... ...1 = 0x01 = 1
		//		numsats available = HH&0x02				= .... ..1. = 0x20 = 1
		// hdop = 3, hdop_available = 1, num_sats_available = 1, low bit of num_sats = 1, numsats available = 1

		int ihdop = roundf(gps_model.hdop);

		uint8_t HH = 0;
		if (gps_model.fix_type > 0)
			HH = 0x80 | ((ihdop & 0x3f) << 2) | 0x01 | 0x02;
		dg[3] = HH;

		dg[4] = 0x00;               // ?? = unknown
		dg[5] = 0x33;               // AA = antenna height; apparenly 0x33 is a constant that is seen in real Raystar GPS devices
		dg[6] = 0x20;               // GG = geoidal separation (32 * 16 = 512 m)
		dg[7] = 0x00;               // ZZ = differential age high bits
		dg[8] = 0x00;               // YY = diff age low bits + flags + station ID high bits
		dg[9] = 0x00;               // DD = station ID low

		queueDatagram(0,dg);
	}


	//-----------------------------------
	// SAT_DETAIL1..4 && SATS_USED
	//-----------------------------------

	if (gps_model.year > 0)
	{
		int num_total = 0;
		int num_used = 0;
		initStSatMessages();

		display(dbg_st_out+1,"neoGPS sending st_sat_details",0);
		proc_entry();

		for (int prn_m1=0; prn_m1<MAX_PRN && num_total < ST_MAX_VIEW; prn_m1++)
		{
			gps_sat_t *sat = &gps_model.sats[prn_m1];
			if (!sat->elev) continue;		// SEEN
			num_total++;

			bool used = sat->flags & SAT_USED_IN_SOLUTION;
			bool viewed = sat->flags & SAT_IN_VIEW;

			if (used)	// prevent used overflow
			{
				if (num_used<ST_MAX_TRACKED)
					num_used++;
				else
					used = 0;
			}

			// map snr to 0 for E80 to show "search" if !viewed and !used
			uint8_t snr = sat->snr;
			if (!used && !viewed) snr = 0;

			// add it
			display(dbg_st_out+2,"neoGPS sending prn(%d) elev(%d) asim(%d) snr(%d) used(%d)",
				prn_m1+1, sat->elev, sat->azim, snr, used);
			addStSatMessage(prn_m1+1, sat->elev, sat->azim, snr, used?2:0);
		}
		proc_leave();

		// send all 5 datagrams

		queueStSatMessages(0);
	}


	//----------------------------------
	// SATS_DONE
	//----------------------------------

	if (gps_model.year > 0)
	{
		display(dbg_st_out+1,"neoGPS sending st_sats_done",0);
		dg[0] = ST_SAT_DETAIL;
		dg[1] = 0x98;
		dg[2] = 0;
		dg[3] = 0;
		dg[4] = 0;
		dg[5] = 0;
		dg[6] = 0;
		dg[7] = 0;
		dg[8] = 0;
		dg[9] = 0;
		dg[10] = 0;
		queueDatagram(0,dg);
	}


	//------------------------------------------
	// LATLON
	//------------------------------------------

	if (1)
	{
		double lat = gps_model.lat;
		double lon = gps_model.lon;
		display(dbg_st_out+1,"neoGPS sending LatLon(%0.6f,%0.6f)",lat,lon);

		uint8_t Z1 = 0;
		uint8_t Z2 = 0x20;
		if (lat < 0)
		{
			lat = abs(lat);
			Z1 = 0x10;
		}
		if (lon < 0)
		{
			lon = abs(lon);
			Z2 = 0x0;
		}

		// integer portions
		uint16_t i_lat = lat;
		uint16_t i_lon = lon;

		// right of decimal point
		float frac_lat = lat - i_lat;
		float frac_lon = lon - i_lon;

		// converted to minutes
		float min_lat = frac_lat * 60.0;
		float min_lon = frac_lon * 60.0;

		// times 1000 into integers
		int imin_lat = round(min_lat * 1000.0);
		int imin_lon = round(min_lon * 1000.0);

		proc_entry();
		display(dbg_st_out+1+1,"i_lat(%d) frac_lat(%0.6f) min_lat(%0.6f) imin_lat(%d)",i_lat,frac_lat,min_lat,imin_lat);
		display(dbg_st_out+1+1,"i_lon(%d) frac_lon(%0.6f) min_lon(%0.6f) imin_lon(%d)",i_lon,frac_lon,min_lon,imin_lon);
		proc_leave();

		dg[0] = ST_LATLON;					// 0x158
		dg[1] = 0x5 | Z1 | Z2;
		dg[2] = i_lat;
		dg[3] = (imin_lat >> 8) & 0xff;
		dg[4] = imin_lat & 0xff;
		dg[5] = i_lon;
		dg[6] = (imin_lon >> 8) & 0xff;
		dg[7] = imin_lon & 0xff;
		queueDatagram(0,dg);
	}


	//------------------------------------------
	// COG/SOG
	//------------------------------------------

	if (gps_model.year > 0)
	{
		if (gps_model.cog >= 0)
		{
			// float degrees = gps_model.cog;
			float degrees = boat_sim.makeMagnetic(gps_model.cog);

			int halfs_total = roundf(degrees * 2.0);
			int nineties = halfs_total / 180;
			int rem = halfs_total % 180;
			int twos = rem / 4;
			int halfs = rem % 4;

			display(dbg_st_out+1,"neoGPS sending COG(%0.1f) = nineties(%d) twos(%d) halfs(%d)",degrees,nineties,twos,halfs);

			dg[0] = ST_COG;		// 0x153
			dg[1] = 0 | (nineties << 4) | (halfs<<6);
			dg[2] = twos;
			queueDatagram(0,dg);
		}

		if (gps_model.sog >= 0)
		{
			double speed = gps_model.sog ;
			int ispeed = (speed+ 0.05) * 10;
			display(dbg_st_out+1,"neoGPS sending SOG(%0.1f)",speed);

			dg[0] = ST_SOG;		// 0x152
			dg[1] = 0x01;
			dg[2] = ispeed & 0xff;
			dg[3] = (ispeed >> 8) & 0xff;
			queueDatagram(0,dg);
		}
	}


	//------------------------------------------
	// DATE and TIME
	//------------------------------------------

	if (gps_model.year > 0)
	{
		int y = gps_model.year % 100;
		int m = gps_model.month;
		int d = gps_model.day;

		display(dbg_st_out+1,"neoGPS sending Date(%02d/%02d/%02d)",y,m,d);
		dg[0] = ST_DATE;			// 0x156
		dg[1] = 0x01 | (m << 4);
		dg[2] = d;
		dg[3] = y;
		queueDatagram(0,dg);

		// RST is 12 bits (6 bits for minute, 6 bits for second)
		// T is four bits (low order four bits of second)
		// RS is eight bits (6 bits of minute followed by 2 bits of second)

		int s = gps_model.seconds;
		int h = gps_model.hour;
		int mm = gps_model.minute;

		uint16_t RST = (mm << 6) | s;
		uint16_t T = RST & 0xf;
		uint16_t RS = RST >> 4;

		display(dbg_st_out+1,"neoGPS sending Time(%02d:%02d:%02d)",h,mm,s);
		dg[0] = ST_TIME;				// 0x154
		dg[1] = 0x01 | (T << 4);
		dg[2] = RS;
		dg[3] = h;
		queueDatagram(0,dg);
	}

	proc_leave();

}	// sendNeoST()



//-------------------------------------------------------------
// Minimal ST parser for DEV_QUERY and Restart GPS Button
//------------------------------------------------------------
// Restart GPS button


static void replyToRestartGPSButton()
	// Called from instST_in.cpp while parsing "in" SAT_DETAIL
	// and DIF_DETAIL messages that are not otherwise expected.
{
	warning(0,"replyToRestartGPSButton()",0);

	// This very specific signature and response allows the "Restart GPS" button
	// to "work" as a signal to this code which normally sends:
	//
	//		ST2_SAT_DETAIL  a5 4d 00 00 00 00 00 00 00 00 00 00 00 00 00 08
	//      ST2_DIF_DETAIL  a7 06 ff `ff 07 00 00 00 fe
	//
	// We do not retspond to the DIF_DETAIL message(s)
	// We will get a number of messages until the system calms down,
	// including some 0x1a5 4d's, and if we respond to them, we create
	// an endless loop of them.
	//
	// Therefore clients are careful to call this method except in response
	// to this one specific signature
	//  	dg[0] == 0xa5 && dg[1]==0x4d && dg[15] == 0x08

	uint16_t out_dg[MAX_ST_BUF];

	// minimal low order reply determined empirically

	out_dg[0] = 0x1a5;		// ST_SAT_DETAIL = 0x1a5
	out_dg[1]  = 0x4d;
	out_dg[2]  = 0x00;
	out_dg[3]  = 0x80;	// 0x80 works
	out_dg[4]  = 0x00;
	out_dg[5]  = 0x00;
	out_dg[6]  = 0x00;
	out_dg[7]  = 0x00;
	out_dg[8]  = 0x00;
	out_dg[9]  = 0x00;
	out_dg[10] = 0x00;
	out_dg[11] = 0x00;
	out_dg[12] = 0x00;
	out_dg[13] = 0x00;
	out_dg[14] = 0x00;
	out_dg[15] = 0x08;
		// 0x08, 0x10, and 0x18 all "works" to satisfy the Restart GPS button.
		// 		all of these show the default "Mode: Non-Differential"
		// If the 0x04 bit added it shows "Mode: Automatic Differential"
		// 		and then subsequently returns 0x0c (not 0x08) in the request
		//		which blows are signature.

	clearSTQueues();
	queueDatagram(0,out_dg);
	sendDatagram(0,ST_SERIAL);
}



static void parseDatagram(uint8_t *dg)
{
	if (!seatalk_enabled) return;
	
	if (dg[0] == 0xa4 &&	// ST_DEV_QUERY == 0x1a4
	   (dg[1] == 0x06 || dg[1] == 0x02))
	{
		warning(dbg_st_in,"DEV_QUERY",0);
		st_neo_device_query_pending = 1;
	}
	else if (
		dg[0] == 0xa5 && 	// ST_SAT_DETAIL = 0x1a5
		dg[1] == 0x4d &&
		dg[15] == 0x08)
	{
		warning(dbg_st_in,"RESTART GPS",0);
		replyToRestartGPSButton();

		// reboot teensyGPS after giving a delay for ST message to be received
		delay(300);
		warning(0,"REBOOTING teensyGPS!!",0);
		delay(300);
		SCB_AIRCR = 0x05FA0004;
		while (1) { delay(1000); }
	}
}



// extern
void handleStPort()
{

	static uint32_t last_st_read;
	static uint32_t last_st_write;
	static int outp = 0;
	static int dlen = 0;
	static uint8_t datagram[MAX_ST_BUF];

	// purge the input buffer

	while (ST_SERIAL.available())
	{
		int c = ST_SERIAL.read();
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
		sendDatagram(0,ST_SERIAL);
		last_st_write = millis();
	}
}




// end of neoST.cpp
