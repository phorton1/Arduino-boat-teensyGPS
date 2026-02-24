//----------------------------------------
// neo6M_GPS.cpp
//----------------------------------------
// The neo6m sends a burst of NMEA0183 messages every second.
// We "frame" the burst as a "cycle" based on an idle time of at least 50ms
//
// IMPORTANT NOTE REGARDING PRNS
//		GPS uses 1..32
//		GLONASS 1–24
//		Galileo uses 1–36
//		BeiDou uses 1–63
//		SBAS uses 120–158
//		QZSS uses 193–199
//
// WE ARE ONLY USING GPS AND OUR ARRAY IS INDEXED BY PRN-1

#include <myDebug.h>
#include "neoGPS.h"
#include <TimeLib.h>


#define dbg_neo			0			// lifecycle
#define dbg_raw			1			// 0 = show raw 0183 messages
#define dbg_0183		1			// 0,-1 = show parseNeo0183 info

#define DBG_STATUS		1			// show msg every 100 parses + status advances



//--------------------------
// Module
//--------------------------

#define FRAME_IDLE_TIME	50			// 50 ms idle defines a frame
	// we skip first, possibly partial, frame after initialization

gps_model_t gps_model;

static uint32_t last_receive_time = 0;
static bool neo_started = 0;
	// skip one frame to start

static int got_gsa = 0;
#if DBG_STATUS
	static int last_fix     = -1;
	static int last_view    = -1;
#endif


static void initModel()
{
	memset(&gps_model,0,sizeof(gps_model));
    gps_model.fix_type      = -1;				// must be 0..3 in NMEA0183
	gps_model.lat           = 0.00;				// ST just sends zeros until fix
	gps_model.lon           = 0.00;
	gps_model.altitude      = 0.00;
	gps_model.hdop          = -1;
	gps_model.vdop          = -1;
	gps_model.pdop          = -1;
	gps_model.sog           = -1;
	gps_model.cog           = -1;
	gps_model.num_viewed 	= -1;
    gps_model.num_used      = -1;
    gps_model.year          = -1;
    gps_model.month         = -1;
    gps_model.day           = -1;
    gps_model.hour          = -1;
    gps_model.minute        = -1;
	gps_model.seconds       = -1;

	got_gsa = 0;

	#if DBG_STATUS
		last_fix     = -1;
		last_view    = -1;
	#endif
};


static void initCycle()
{
	got_gsa = 0;
	for (int i=0; i<MAX_PRN; i++)
	{
		gps_model.sats[i].flags = 0;
	}
}



//-----------------------------------------------------------
// initialization
//-----------------------------------------------------------

static bool genuineNeoModule()
	// Ironically, we dont need to configure a genuine module
	// to not send GLONASS and Bedieu messages, and we CANNOT
	// configure a clone to not send them.
{
    display(0,"Requesting UBX-MON-VER...",0);
	const uint8_t ubx_mon_ver_request[] = {
		0xB5,0x62,        // UBX header
		0x0A,0x04,        // MON-VER
		0x00,0x00,        // length = 0
		0x0E,0x34         // checksum
	};

		NEO_SERIAL.write(ubx_mon_ver_request, sizeof(ubx_mon_ver_request));

    uint32_t start = millis();
    int state = 0;

	#define MAX_VER_BUF  255
	static char buf[MAX_VER_BUF+1];
	uint16_t len = 0;
    uint16_t count = 0;

    while (1)
	{
		if (millis() - start > 300)
		{
			my_error("genuineNeoModule() UBX-MON_VER timeout",0);
			return false;
		}

        if (!NEO_SERIAL.available()) {
            yield();
            continue;
        }

        uint8_t b = NEO_SERIAL.read();

        switch (state) {

            case 0: // hunt for 0xB5
                if (b == 0xB5) state = 1;
                break;

            case 1: // hunt for 0x62
                if (b == 0x62) state = 2;
                else state = 0;
                break;

            case 2: // class
                if (b != 0x0A)
				{
					my_error("genuineNeoModule() expected UBX class(0x0a) got 0x%02x",b);
					return false;
				}
                state = 3;
                break;

            case 3: // id
                if (b != 0x04)
				{
					my_error("genuineNeoModule() expected UBX id(0x04) got 0x%02x",b);
					return false;
				}
                state = 4;
                break;

            case 4: // length LSB
                len = b;
                state = 5;
                break;

            case 5: // length MSB
                len |= (uint16_t)b << 8;
				if (len > MAX_VER_BUF)
				{
					my_error("genuineNeoModule() expected len<%d, got %d",MAX_VER_BUF,len);
					return false;
				}
				state = 6;
				break;

            case 6: // read MON-VER payload
                if (dbg_neo<=0)
					Serial.write(b);
				buf[count++] = b;

                if (count == len)
				{
					buf[len] = 0;
					if (dbg_neo<=0)
						Serial.println();

					// return TRUE if genuine neo module
					// apparently the chinese were at least kind enough to not
					// fake the genuine signature that looks something like
					//
					//		SW VERSION: 7.03 (45969)
					//		HW VERSION: 00040007
					//		EXT CORE 2.00 ...

					if (strstr(buf, "SW VERSION:"))
						return true;   // genuine u-blox
					else
						return false;  // clone
				}
				break;
        }
    }
}





void initNeoGPS()
{
	display(dbg_neo,"initNeoGPS(%s,%s)",
		seatalk_enabled?"SEATALK":"-",
		nmea2000_enabled?"NMEA2000":"-");
	proc_entry();

	#if 1
		// Allocate a larger RX buffer for NEO_SERIAL
		display(dbg_neo,"increasing NEO_SERIAL buffer size",0);

		#define NUMBER_BUF_ELEMENTS 10240
		static uint16_t serial1_rxbuf[NUMBER_BUF_ELEMENTS];
		// And the use of uint8_t for the buffer is not correct
		// within the context of the SERIAL_9BIT_SUPPORT:
		// 		static uint8_t serial1_rxbuf[NUMBER_BUF_ELEMENTS];
		NEO_SERIAL.addMemoryForRead(serial1_rxbuf, NUMBER_BUF_ELEMENTS);
		// This causes hard crashes:
		// 		NEO_SERIAL.addMemoryForRead(serial1_rxbuf, sizeof(serial1_rxbuf));
	#endif

	neo_started = 0;
	last_receive_time = 0;
	initModel();

	NEO_SERIAL.begin(9600);
	display(dbg_neo,"NEO_SERIAL started",0);
	delay(300);

	if (genuineNeoModule())
		warning(dbg_neo,"GENUINE uBLOX NEO6M MODULE FOUND",0);
	else
		warning(dbg_neo,"CLONE NEO6M MODULE!!",0);

	proc_leave();
	display(dbg_neo,"initNeo6M_GPS() finished",0);
}




//-----------------------------------------------------------
// NMEA0183 Parser
//-----------------------------------------------------------

static int hexval(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return -1;
}



static bool checkOK(const char *msg)
	// Return true if checksum matches, false otherwise
{
    // get transmitted checksum (two hex chars)

    const char *star = strchr(msg, '*');
    if (!star || !star[1] || !star[2])
        return false;
    int hi = hexval(star[1]);
    int lo = hexval(star[2]);
    if (hi < 0 || lo < 0)
        return false;
    uint8_t chk = (hi << 4) | lo;

    // Compute XOR of all chars between '$' and '*'

    uint8_t sum = 0;
    const char *p = msg;
    if (*p == '$')
        p++;
    while (p < star)
    {
        sum ^= (uint8_t)(*p);
        p++;
    }

	if (sum != chk)
	{
		my_error("neo6M_GPS CSERR exp=0x%02x calc=0x%02x len=%d",
			chk, sum, (int)strlen(msg));
		display_bytes(0,"ERR",(const uint8_t *)msg,strlen(msg));
		warning(0,"CSERR MSG=%s",msg);
		return false;
	}
    return true;
}


static int tokenize0183(const char *msg, String *out, int max)
{
    int count = 0;
    const char *p = msg;
    while (*p && count < max)
    {
        const char *start = p;
        while (*p && *p != ',' && *p != '*')
            p++;

        out[count++] = String(start).substring(0, p - start);

        if (*p == ',')
            p++;
        else if (*p == '*')
            break;
    }
    return count;
}


static double getLat0183(const String &val, const String &ns)
{
    if (!val.length() || !ns.length()) return 0.0;
    double raw = val.toFloat();          // ddmm.mmmm
    int deg = (int)(raw / 100);
    double minutes = raw - deg * 100;
    double lat = deg + minutes / 60.0;
    if (ns == "S") lat = -lat;
    return lat;
}

static double getLon0183(const String &val, const String &ew)
{
    if (!val.length() || !ew.length()) return 0.0;
    double raw = val.toFloat();          // dddmm.mmmm
    int deg = (int)(raw / 100);
    double minutes = raw - deg * 100;
    double lon = deg + minutes / 60.0;
    if (ew == "W") lon = -lon;
    return lon;
}



static void parseNeo0183(const char *msg)
{
	display(dbg_raw, "parseNeo0183: %s", msg);
	if (!checkOK(msg))
		return;

    String tok[20];
    int num_toks = tokenize0183(msg, tok, 20);
    if (num_toks < 0)
	{
		if (num_toks == 0)
			warning(dbg_0183,"neo6M_GPS empty message",0);
		return;
	}


	//------------------
	// messages
	//------------------

	proc_entry();

    if (tok[0].endsWith("GGA"))
    {
        // fix type gotten preferentially from GSA
		if (tok[7].length()) gps_model.num_used = tok[7].toInt();
        if (tok[8].length()) gps_model.hdop = tok[8].toFloat();
        if (tok[9].length()) gps_model.altitude = tok[9].toFloat();

        display(dbg_0183, "GGA fix=%d used=%d hdop=%.1f alt=%.1f",
			gps_model.fix_type, gps_model.num_used,
			gps_model.hdop, gps_model.altitude);
    }
    else if (tok[0].endsWith("GSA"))
    {
		if (num_toks != 19)
		{
			my_error("MALFORMED GSA Sentence: %s",msg);
			proc_leave();
			return;
		}

		// the clone neo apparently sends out an empty GSA for GLONAS, and in
		// addition it may send another GSA for some other constellation with
		// a satellite. We accept only the first GSA per cycle.

		if (got_gsa)
		{
			// Serial.println("    skipGSA");
			proc_leave();
			return;
		}
		got_gsa++;

		// pdop is required by NMEA0183 spec but we treat it as optional

        gps_model.fix_type = tok[2].toInt();
		if (tok[15].length()) gps_model.pdop = tok[15].toFloat();
        if (tok[16].length()) gps_model.hdop = tok[16].toFloat();
        if (tok[17].length()) gps_model.vdop = tok[17].toFloat();

		// mark used PRNs

		int b_ptr = 0;
		char used_buf[120];		// debugging only
		for (int i=3; i<=14 && i<num_toks; i++)
		{
			if (!tok[i].length()) continue;
			int prn = tok[i].toInt();
			if (prn<=0 || prn>MAX_PRN)		// and it defintitely should not be 0 or negative
			{
				my_error("INVALID GSA PRN(%d)!!!!!",prn);
				continue;
			}
			gps_model.sats[prn-1].flags |= SAT_USED_IN_SOLUTION;
			sprintf(&used_buf[b_ptr*3]," %02d",prn);
			b_ptr++;
		}
		used_buf[b_ptr*3] = 0;

        display(dbg_0183, "GSA fix=%d pdop=%s hdop=%s vdop=%s used: %s",
            gps_model.fix_type,
			tok[15].c_str(),
			tok[16].c_str(),
			tok[17].c_str(),
			used_buf);
		

		#if DBG_STATUS
			// show fix type changes
			if (gps_model.fix_type != last_fix)
			{
				warning(0, "fix_type %d->%d at %lu",
					last_fix, gps_model.fix_type, millis()/1000);
				last_fix = gps_model.fix_type;
			}
		#endif
    }
    else if (tok[0] == "$GPGSV")		//  tok[0].endsWith("GSV"))
    {
		// Note that we are explicitly just "skipping" $BDGSV,
		// the neo's view of Beidou satellites with the idea that
		// (a) it prioritizes use of the GPS constellation
		// (b) the E80 only expects a GPS constellation
		// (c) if there are no bars or icons on the e80 Sat status dialog
		//     it is not really important, the only thing that really
		//	   matters is the lat/lon
		
        int total_msgs = tok[1].length() ? tok[1].toInt() : -1;		// used from this message to copy to gps_model
        int msg_num    = tok[2].length() ? tok[2].toInt() : -1;		// used from this message to copy to gps_model
        if (tok[3].length())										// used only for debugging
			gps_model.num_viewed = tok[3].toInt();

        display(dbg_0183, "GSV %d/%d ideal num_viewed=%d",
            msg_num, total_msgs, gps_model.num_viewed);

		// Loop through the PRNS presented in this GSV message and
		// set their SAT_IN_VIEW bits and elev/azim/snr if provided with
		// notion that we are maintaining old elev/azim/snr's as the
		// last known values for satellites that come in and go out ov view

        proc_entry();
        for (int i = 0; i < 4; i++)
        {
            int idx = 4 + i * 4;			// the index of the ith' satellites TOKEN in the message
            if (idx + 3 >= num_toks)		// which might not exists for the last (shortened) message
                break;						// in which case we terminate the loop

			int prn = tok[idx].toInt();		// get the prn, where ,, might, but shouldn't == 0
			if (prn<=0 || prn>MAX_PRN)		// and it defintitely should not be 0 or negative
			{
				// coPilot says PRNSs > 32 are likely GLONASS and
				// can be ignored.  GLONASS can be disable by UBX protocol,
				// but the larger issues is that UBX protocol is probably
				// "better" than NMEA0183 for using the neo6m.

				warning(dbg_0183+1,"INVALID GSV PRN(%d)!!!!!",prn);
				continue;
			}

			gps_model.sats[prn-1].flags |= SAT_IN_VIEW;

			// assign any elev,asim, or snr that is not ,,

			if (tok[idx+1].length())
				gps_model.sats[prn-1].elev = tok[idx+1].toInt();
			if (tok[idx+2].length())
				gps_model.sats[prn-1].azim = tok[idx+2].toInt();
			if (tok[idx+3].length())
				gps_model.sats[prn-1].snr  = tok[idx+3].toInt();

            display(dbg_0183+1, "sat[%02d] prn(%d) elev=%s az=%s snr=%s",
				i,
				prn,
				tok[idx+1].c_str(),
				tok[idx+2].c_str(),
				tok[idx+3].c_str());
        }

        proc_leave();

		#if DBG_STATUS
			// show increasing number of sats in view
			if (gps_model.num_viewed > last_view)
			{
				warning(0, "num_viewed %d->%d at %lu",
					last_view, gps_model.num_viewed, millis()/1000);
				last_view = gps_model.num_viewed;
			}
		#endif
    }

	else if (tok[0].endsWith("RMC"))
	{
		// time: hhmmss.ss
		int got_dt = 0;

		if (tok[1].length() >= 6)
		{
			int hh = tok[1].substring(0, 2).toInt();
			int mm = tok[1].substring(2, 4).toInt();
			int ss = tok[1].substring(4, 6).toInt();
			#if DBG_STATUS
				if (gps_model.year == -1)
				{
					warning(0,"got time(%02d:%02d:%02d)",hh,mm,ss);
				}
			#endif
			gps_model.hour   = hh;
			gps_model.minute = mm;
			gps_model.seconds = ss;
			got_dt++;
		}

		// date: ddmmyy
		if (tok[9].length() == 6)
		{
			int dd = tok[9].substring(0, 2).toInt();
			int mo = tok[9].substring(2, 4).toInt();
			int yy = tok[9].substring(4, 6).toInt() + 2000;
			#if DBG_STATUS
				if (gps_model.year == -1)
				{
					warning(0,"got date(%d-%02d-%02d)",yy,mo,dd);
				}
			#endif
			gps_model.day   = dd;
			gps_model.month = mo;
			gps_model.year  = yy;
			got_dt++;

		}

		if (got_dt == 2)
		{
			// set the teensy's clock if got both date and time
			
			setTime(gps_model.hour,
					gps_model.minute,
					gps_model.seconds,
					gps_model.day,
					gps_model.month,
					gps_model.year);
		}

		if (tok[3].length() && tok[4].length())
			gps_model.lat = getLat0183(tok[3], tok[4]);
		if (tok[5].length() && tok[6].length())
			gps_model.lon = getLon0183(tok[5], tok[6]);

		if (tok[7].length()) gps_model.sog = tok[7].toFloat();
		if (tok[8].length()) gps_model.cog = tok[8].toFloat();

		display(dbg_0183,
			"RMC lat=%.5f lon=%.5f sog=%.1f cog=%.1f %04d-%02d-%02d %02d:%02d:%02d",
			gps_model.lat, gps_model.lon, gps_model.sog, gps_model.cog,
			gps_model.year, gps_model.month, gps_model.day,
			gps_model.hour, gps_model.minute, gps_model.seconds);
	}

	proc_leave();

}	// parseNeo0183()




//---------------------------------------------------
// doNeoGPS
//---------------------------------------------------

void doNeoGPS()
	// called from loop()
	// extern'd in instSimulator.h
{
	while (NEO_SERIAL.available())
	{
		#define MAX_0183_MSG 180

		int c = NEO_SERIAL.read();
		static char buf[MAX_0183_MSG+1];
		static volatile int buf_ptr = 0;
		last_receive_time = millis();

		if (neo_started)
		{
			if (buf_ptr >= MAX_0183_MSG)
			{
				buf[buf_ptr] = 0;
				my_error("NEO_TOO_LONG: `%s'",buf);
				buf_ptr = 0;
			}
			else if (c == '\n')
			{
				buf[buf_ptr] = 0;
				parseNeo0183(buf);
				buf_ptr = 0;
				return;
			}
			else if (c != '\r')
			{
				if (buf_ptr && c == '$')
				{
					buf[buf_ptr] = 0;
					my_error("NEO 2nd dollar after: `%s'",buf);
					buf[0] = c;
					buf_ptr = 1;
				}
				buf[buf_ptr++] = c;
			}
		}
	}


	static int cycle_count;

	if (last_receive_time &&
		millis() - last_receive_time > FRAME_IDLE_TIME)
	{
		if (neo_started)
		{
			#if DBG_STATUS

				#define SHOW_EVERY		10

				if (cycle_count % SHOW_EVERY == 0)
				{
					Serial.print("neo status secs(");
					Serial.print(millis()/1000);
					Serial.print(") fix(");
					Serial.print(gps_model.fix_type);
					Serial.print(") num_viewed(");
					Serial.print(gps_model.num_viewed);
					Serial.print(") num_used(");
					Serial.print(gps_model.num_used);
					Serial.print(") hdop(");
					Serial.print(gps_model.hdop);
					Serial.print(") year(");
					Serial.print(gps_model.year);
					Serial.print(") cog(");
					Serial.print(gps_model.cog);
					Serial.print(") sog(");
					Serial.print(gps_model.sog);
					Serial.println(")");
		
					// a PRN has been "seen" if it's elev is > 0, which defines the "almanac" for the neo
					//
					// the sats IN_VIEW bit is cleared when we start a GSV cycle, the end of which causes a sendNeo()
					//		GSV sets the az,ele, and snr values for a satellite if provided
					//
					// the sats USED bit is set by a GSA message that occurs once per cycle, before, or after the GSV,
					//		but is cleared at the end of the cycle after calling sendNeo()
					//

					Serial.print("    Seen Sats: ");
					for (int i=0; i<MAX_PRN; i++)
					{
						if (gps_model.sats[i].elev)
						{
							Serial.print(i+1);
							Serial.print(" ");
						}
					}
					Serial.println();
					Serial.print("    View Sats: ");
					for (int i=0; i<MAX_PRN; i++)
					{
						if (gps_model.sats[i].flags & SAT_IN_VIEW)
						{
							Serial.print(i+1);
							Serial.print(" ");
						}
					}
					Serial.println();
					Serial.print("    Used Sats: ");
					for (int i=0; i<MAX_PRN; i++)
					{
						if (gps_model.sats[i].flags & SAT_USED_IN_SOLUTION)
						{
							Serial.print(i+1);
							Serial.print(" ");
						}
					}
					Serial.println();
				}
			#endif

			cycle_count++;
			if (seatalk_enabled > 1)
				sendNeoST();
			if (nmea2000_enabled > 1)
				sendNeo2000();
		}
		else
		{
			display(dbg_neo,"skipping 0th cycle",0);
			cycle_count = 0;
		}
		
		initCycle();
		last_receive_time = 0;
		if (!neo_started)
			warning(dbg_neo,"NEO STARTED",0);
		neo_started = 1;
	}
}




// end of neo6M_GPS.cpp
