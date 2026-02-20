//----------------------------------------
// neo2000.cpp
//----------------------------------------

#include <myDebug.h>
#include "neoGPS.h"
#include "inst2000.h"
#include <N2kMessages.h>
#include <TimeLib.h>

#define dbg_neo_2000	1			// 0,-1 = show sent 2000 pgns

#define DBG_STATUS		1			// show msg every 100 parses + status advances


// Sends the same messages in the same order as inst2000_out.cpp
// gpsInst::send2000() which is known to work with the E80
// GPS status window to show bars and icons.
//
// MY invalid values are -1.
// Here I provide methods to map my invalid values to the
// NMEAInvalid values as needed.


static double mapDoubleNA(double v)
{
	return (v < 0.0) ? N2kDoubleNA : v;
}

static uint8_t mapUInt8NA(int v)
{
	return (v < 0) ? N2kUInt8NA : (uint8_t)v;
}

static double mapDegToRadNA(double v)
	// NOT USED BELOW FOR lat/lon/altituged which were
	// initialized to NA
{
	return (v < 0) ? N2kDoubleNA : DegToRad(v);
}


// extern
void sendNeo2000()
	// We use gps_model.year>0 both as a flag that the year is valid for
	// PGN 126992, but perhaps more importantly, as a flag that the
	// entire gps_model data structure is valid for PGN 129540
{
	tN2kMsg msg;

	int actual_num_viewed = 0;
	int actual_num_used = 0;
	for (int i=0; i<MAX_PRN; i++)
	{
		if (gps_model.sats[i].flags & SAT_IN_VIEW)
			actual_num_viewed++;
		if (gps_model.sats[i].flags & SAT_USED_IN_SOLUTION)
			actual_num_used++;
	}

	// PGN 129029 - GNSS Position Data

	if (gps_model.fix_type >= 1)
	{
		// teensy TimeLib.h has already been updated in the RMC parser:
		// ^^^ this may be a bad assumption since fix_type is not from RMC,
		// but at least they're valid values (not NA) in all cases below

		time_t t = now();
		uint32_t days = t / 86400;
		uint32_t secs = t % 86400;

		display(dbg_neo_2000,
			"129029 GNSS lat=%.6f lon=%.6f alt=%.1f fix=%d actual_num_used=%d hdop=%.1f vdop=%.1f pdop=%.1f",
			gps_model.lat,
			gps_model.lon,
			gps_model.altitude,
			gps_model.fix_type,
			actual_num_used,
			gps_model.hdop,
			gps_model.vdop,
			gps_model.pdop
		);

		SetN2kPGN129029(msg,
			255,							// SID (sequence ID)
			days,							// Days since 1970-01-01	gotten from teensy clock, whatever that is
			secs,							// Seconds since midnight	gotten from teensy clock, whatever that is
			gps_model.lat,					// Latitude (deg)			already mapped to NA if un-inited
			gps_model.lon,					// Longitude (deg)			already mapped to NA if un-inited
			gps_model.altitude,				// Altitude (m)				already mapped to NA if un-inited
			N2kGNSSt_GPS,					// GNSS type (GPS)
			N2kGNSSm_GNSSfix,				// GNSS method (GNSS fix)
			actual_num_used,				// Number of satellites used
			mapDoubleNA(gps_model.hdop),		// HDOP						turned into NA if < 0
			mapDoubleNA(gps_model.pdop),		// PDOP						turned into NA if < 0
			0,								// Geoidal separation (m)
			0,								// Position accuracy estimate (m)
			N2kGNSSt_GPS,					// Integrity type
			0,								// Reserved
			0 );							// Reserved
		nmea2000.SendMsg(msg);
	}


	// inst2000 instGPS sends PGN_DIRECTION_DATA 130577 here
	// we only send sog and cog, we don't presume to know the heading
	// or anything about the water speed, set, or drift

	if (gps_model.year > 0 && (gps_model.sog >= 0 || gps_model.cog >= 0))		// valid DT from GPS
	{
		// PGN_DIRECTION_DATA

		double sog_mps = gps_model.sog < 0 ? N2kDoubleNA : KnotsToms(gps_model.sog);

		SetN2kPGN130577(msg, 					// msg
			N2kDD025_Estimated,					// tN2kDataMode
			N2khr_true,							// tN2kHeadingReference,
			255,								// sid,
			mapDegToRadNA(gps_model.cog),			// COG in radians
			sog_mps,							// SOG in m/s
			N2kDoubleNA,						// heading in radians
			N2kDoubleNA,						// speed through water in m/s
			N2kDoubleNA,						// Set
			N2kDoubleNA);						// Drift
		nmea2000.SendMsg(msg);
	}

	if (gps_model.year > 0)		// valid DT from GPS
	{
		// PGN 126992 - System Time

		time_t now = time(NULL);
		uint32_t days = now / 86400;
		uint32_t secs = now - days * 86400;

		display(dbg_neo_2000,
			"126992 Time %04d-%02d-%02d %02d:%02d:%.1f",
			gps_model.year,
			gps_model.month,
			gps_model.day,
			gps_model.hour,
			gps_model.minute,
			gps_model.seconds
		);

		SetN2kPGN126992(msg,
			255,
			days,
			secs,
			N2ktimes_GPS
		);
		nmea2000.SendMsg(msg);
	}


	if (gps_model.year > 0)	// Valid (if partly unitialized) gps_model record from a cycle
	{
		// PGN 129540 - Satellites in View
		// we count the sats in view based on the SAT_IN_VIEW bit, and
		// only emit this message if the number is nonzero

		if (actual_num_viewed > 0)
		{
			display(dbg_neo_2000,"129540 SatsInView actual_num_viewed(%d)",actual_num_viewed);
			proc_entry();

			int sid = 7;  // or any fixed non-255 SID
			SetN2kPGN129540(msg,sid,N2kDD072_Unavailable);
				// N2kDD072_Unavailable == the GNSS receiver did not provide any integrity/differential status information

			for (int prn_m1=0; prn_m1<MAX_PRN; prn_m1++)
			{
				if (!(gps_model.sats[prn_m1].flags & SAT_IN_VIEW))
					continue;  // not in most recent GSV
				bool used = gps_model.sats[prn_m1].flags & SAT_USED_IN_SOLUTION ? 1 : 0;

				display(dbg_neo_2000 + 1,
					"PGN SAT PRN[%d] elev(%d) azim(%d) snr(%d) used(%d)",
					prn_m1 + 1,
					gps_model.sats[prn_m1].elev,
					gps_model.sats[prn_m1].azim,
					gps_model.sats[prn_m1].snr,
					used);

				tSatelliteInfo sat;
				if (0)
				{
					sat.PRN = prn_m1 + 1;
					sat.Elevation = DegToRad(gps_model.sats[prn_m1].elev);
					sat.Azimuth = DegToRad(gps_model.sats[prn_m1].azim);
					sat.SNR = gps_model.sats[prn_m1].snr;
					sat.RangeResiduals = N2kDoubleNA;
				}
				else
				{
					sat.PRN        = prn_m1 + 1;
					sat.Elevation  = mapDegToRadNA(gps_model.sats[prn_m1].elev);
					sat.Azimuth    = mapDegToRadNA(gps_model.sats[prn_m1].azim);
					sat.SNR        = mapUInt8NA(gps_model.sats[prn_m1].snr);
					sat.RangeResiduals = N2kDoubleNA;
				}
				sat.UsageStatus =  used ?
					N2kDD124_UsedInSolutionWithoutDifferentialCorrections :
					N2kDD124_TrackedButNotUsedInSolution;
				AppendN2kPGN129540(msg, sat);
			}

			nmea2000.SendMsg(msg);
			proc_leave();
		}
	}
}


// end of neo2000.cpp
