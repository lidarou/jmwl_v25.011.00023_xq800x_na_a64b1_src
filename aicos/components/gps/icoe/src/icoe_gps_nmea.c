/************************************************************************ 
* Copyright © 2022, ICOE (Shanghai) Technologies Co.,Ltd. All rights reserved.              *
* Information in this document is provided in connection with ICOE’s  products.             *
* No license, express or implied, by estoppels or otherwise, to any intellectual property   *
* rights is granted by this document. Except as  provided in ICOE’s Terms and               *
* Conditions of Sale for such products, ICOE assumes no liability whatsoever, and           *
* ICOE disclaims any express or implied warranty, relating to sale and/or use of            *
* ICOE products including liability or warranties relating to fitness for a particular      *
* purpose, merchantability, or infringement of any patent, copyright or other               *
* intellectual property right. ICOE products are not intended for use in    medical, life   *
* saving, or life sustaining applications.                                                  * 
* ICOE (Shanghai) Technologies Co.,Ltd may make changes to specifications and               * 
* product descriptions at any time, without notice.                                         * 
* You may obtain copies of documents which have a document number and are                   *
* referenced in this document, or other ICOE literature may be obtained by                  * 
* mailing to xiaopingjiang@icoe-tech.com.                                                                                                       *
************************************************************************/ 

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x15a165
#define LOG_MODULE    AMOD_GNSS

#include <time.h>
#include <ctype.h>
#include <math.h>
#include "icoe_gps_nmea.h"
#include "icoe_gps_os.h"
#include "icoe_gps_hal.h"
#include "icoe_gps_serial.h"

extern GpsCallbacks umGpsCallbacks;
NmeaReader nmeaReader;

#define  MAX_NMEA_TOKENS  32

typedef struct {
    const char*  p;
    const char*  end;
} Token;

typedef struct {
    int     count;
    Token   tokens[MAX_NMEA_TOKENS];
} NmeaTokenizer;

#define SECS_IN_DAY (24*60*60)

static int nmea_tokenizer_init( NmeaTokenizer*  t, const char*  p, const char*  end )
{
    int    count = 0;

    // the initial '$' is optional
    if (p < end && p[0] == '$')
        p += 1;

    // remove trailing newline
    if (end > p && end[-1] == '\n') {
        end -= 1;
        if (end > p && end[-1] == '\r')
            end -= 1;
    }

    // get rid of checksum at the end of the sentecne
    if (end >= p+3 && end[-3] == '*') {
        end -= 3;
    }
    const char *q = NULL;
    while (p < end) {
        q = p;

        q = memchr(p, ',', end-p);
        if (q == NULL)
            q = end;

        if (count < MAX_NMEA_TOKENS) {
            t->tokens[count].p   = p;
            t->tokens[count].end = q;
            count += 1;
        }

        if (q < end)
            q += 1;

		p = q;
	}

	// $BDGGA,093256.122,3037.511084,N,10402.236998,E,1,05,8.697,483.933,M,0,M,,*6C
	//                                                                         ^^
	// last field empty?
	if ((end[-1] == ',') && (count < MAX_NMEA_TOKENS )) {
		t->tokens[count].p   = end;
		t->tokens[count].end = end;
		count += 1;
	}

    t->count = count;
    return count;
}

static Token nmea_tokenizer_get( NmeaTokenizer*  t, int  index )
{
    Token  tok;
    static const char*  dummy = "";

    if (index < 0 || index >= t->count) {
        tok.p = tok.end = dummy;
    } else
        tok = t->tokens[index];

    return tok;
}


static int str2int( const char*  p, const char*  end )
{
    int   result = 0;
    int   len    = end - p;
    int sign = 1;

    if(len > 10 || len <= 0) {
        return 0;
    }

    if (*p == '-') {
        sign = -1;
        p++;
        len = end - p;
    }
    int  c;
    for ( ; len > 0; len--, p++ )
    {
        if (p >= end) {
            return -1;
        }

        c = *p - '0';
        if ((unsigned)c >= 10) {
            return -1;
        }

        result = result*10 + c;
    }
    return  result * sign;
}

static double str_to_double(const char* s, char *unused)
{  
    (void)unused;
 
    const char*    p     = s;
    double         value = 0.0;
    int            sign  = 1;
    double         factor;

    if (*p == '-') {
        sign = -1;
        p++;
    }

    while (p && isspace((int)*p)) {
        p++;
	}

    while (p && isdigit((int)(*p))) {
        value = value*10 + (*p++ - '0');
    }

    if ( *p == '.' ) {
        factor = 1.;

    p++;
    while(p && isdigit((int)(*p)))
    {
        factor *= 0.1;
        value  += (*p++ - '0') * factor;
        }
    }

    return value*sign;
}

/*
 * this function only calculate the milliseconds since Jan 1, 1970
 * parameter tm is UTC time
 */
time_t icoe_mktime(struct tm *tm)
{
    long totalSecs = 0;
    long sec1;
    long sec2;
    u8 leapYearCnt = 0;
    u8 yearsNotInLeap = 0;
    u16   dayInYear;
    
    int leapYearDaysOfMonthList[] = {
            31,
            29,
            31,
            30,
            31,
            30,
            31,
            31,
            30,
            31,
            30,
            31,
            };
    
    int daysOfMonthList[] = {
            31,
            28,
            31,
            30,
            31,
            30,
            31,
            31,
            30,
            31,
            30,
            31,
    };

    leapYearCnt = (tm->tm_year - 1972) / 4;
    yearsNotInLeap = (tm->tm_year - 1972) % 4 + 2;  /* 1970 and 1971 is not leap year */
    totalSecs = leapYearCnt * 1461 * SECS_IN_DAY; /* seconds in four years */
    if(yearsNotInLeap > 2) {
        sec1 = 366 * SECS_IN_DAY + (yearsNotInLeap - 1) * 365 * SECS_IN_DAY;
    } else {
        sec1 = yearsNotInLeap * 365 * SECS_IN_DAY;
    }

    dayInYear = 0;
    if(yearsNotInLeap != 2) {
        for(int i = 0; i < tm->tm_mon - 1; i ++ ) {
            dayInYear += daysOfMonthList[i];
        }
    } else {
        for(int i = 0; i < tm->tm_mon - 1; i ++ ) {
            dayInYear += leapYearDaysOfMonthList[i];
        }
    }

    dayInYear += (tm->tm_mday - 1);
    sec2 = dayInYear * SECS_IN_DAY + tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;

    totalSecs += sec1 + sec2;

    return totalSecs;

}

static double str2float( const char*  p, const char*  end )
{
    int   len    = end - p + 1;
    char  temp[32];

    if (len == 0) {
        return -1.0;
    }

    if (len >= (int)sizeof(temp))
        return 0.;

    memcpy( temp, p, len );
    temp[len] = 0;
    return str_to_double(temp, NULL);
}

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       N M E A   P A R S E R                           *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/
void nmea_reader_init()
{
    memset(&nmeaReader, 0, sizeof(NmeaReader));

    nmeaReader.pos      = 0;
    nmeaReader.overflow = 0;
    nmeaReader.nmea_mode = POS_MODE_GN;
    nmeaReader.utc_time.tm_mday = -1;
    nmeaReader.utc_time.tm_mon = -1;
    nmeaReader.utc_time.tm_year = -1;
    nmeaReader.fix.flags = 0;
    memset(nmeaReader.sv_list_tmp, 0, GNSS_MAX_SVS*sizeof(GnssSvInfo));
    memset(nmeaReader.sv_list, 0, GNSS_MAX_SVS*sizeof(GnssSvInfo));
}


static int nmea_reader_get_timestamp(NmeaReader*  r, Token  tok, time_t *timestamp)
{
    if (tok.p + 6 > tok.end)
        return -1;

    if (r->utc_time.tm_year < 0) {
        return -1;
    }

    r->utc_time.tm_hour    = str2int(tok.p,   tok.p+2);
    r->utc_time.tm_min     = str2int(tok.p+2, tok.p+4);
    r->utc_time.tm_sec     = (int)str2float(tok.p+4, tok.end);
    r->utc_time.tm_isdst   = -1;

    *timestamp = icoe_mktime(&(r->utc_time));

    return 0;
}

static int nmea_reader_update_time( NmeaReader*  r, Token  tok )
{
    time_t timestamp = 0;
    int ret = nmea_reader_get_timestamp( r, tok, &timestamp);
    if (0 == ret) {
        r->fix.timestamp = (long long)timestamp * 1000;
    }
    return ret;
}

static int nmea_reader_update_cdate( NmeaReader*  r, Token  tok_d, Token tok_m, Token tok_y )
{

    if ( (tok_d.p + 2 > tok_d.end) ||
            (tok_m.p + 2 > tok_m.end) ||
            (tok_y.p + 4 > tok_y.end) )
        return -1;
    

    r->utc_time.tm_mday = str2int(tok_d.p,   tok_d.p+2);
    r->utc_time.tm_mon = str2int(tok_m.p, tok_m.p+2);
    r->utc_time.tm_year = str2int(tok_y.p, tok_y.end+4);

    return 0;
}

static int nmea_reader_update_date( NmeaReader*  r, Token  date, Token  mtime )
{
    Token  tok = date;
    int    day, mon, year;
    /*
    * nmea Date INfo length == 6
    */
    if (tok.p + 6 != tok.end) {
        return -1;
        /* no date info, will use host time in _update_time function
        */
    }
    /* normal case */
    day  = str2int(tok.p, tok.p+2);
    mon  = str2int(tok.p+2, tok.p+4);
    year = str2int(tok.p+4, tok.p+6) + 2000;

    if ((day|mon|year) < 0) {
        return -1;
    }

    r->utc_time.tm_year  = year;
    r->utc_time.tm_mon   = mon;
    r->utc_time.tm_mday   = day;

    return nmea_reader_update_time( r, mtime );
}

#if 0
static int nmea_reader_update_timemap( NmeaReader* r,
        Token       systime_tok,
        Token       timestamp_tok)
{
    int ret;
    time_t timestamp;

    if ( systime_tok.p >= systime_tok.end ||
            timestamp_tok.p >= timestamp_tok.end)
    {
        r->timemap.valid = 0;
        return -1;
    }

    ret = nmea_reader_get_timestamp(r, timestamp_tok, &timestamp);
    if (ret)
    {
        r->timemap.valid = 0;
        return ret;
    }

    r->timemap.valid = 1;
    r->timemap.systime = str2float(systime_tok.p, systime_tok.end);
    r->timemap.timestamp = (GpsUtcTime)((long long)timestamp * 1000);
    return 0;
}
#endif

extern double floor(double);
static double convert_from_hhmm( Token  tok )
{
    double  val     = str2float(tok.p, tok.end);
    int     degrees = (int)(floor(val) / 100);
    double  minutes = val - degrees*(double)100.0;
    double  dcoord  = degrees + minutes / (double)60.0;
    return dcoord;
}

static int nmea_reader_update_latlong( NmeaReader*  r,
        Token        latitude,
        char         latitudeHemi,
        Token        longitude,
        char         longitudeHemi )
{
    double   lat, lon;
    Token    tok;

    tok = latitude;
    if (tok.p + 6 > tok.end) {
        return -1;
    }
    lat = convert_from_hhmm(tok);
    if (latitudeHemi == 'S')
        lat = -lat;

    tok = longitude;
    if (tok.p + 6 > tok.end) {
        return -1;
    }
    lon = convert_from_hhmm(tok);
    if (longitudeHemi == 'W')
        lon = -lon;

    r->fix.latitude  = lat;
    r->fix.longitude = lon;
    r->fix.flags    |= GPS_LOCATION_HAS_LAT_LONG;
    return 0;
}


static int nmea_reader_update_altitude( NmeaReader*  r,
        Token        altitude,
        Token        hem )
{
    Token   tok = altitude;

    if (tok.p >= tok.end)
        return -1;
    
    r->fix.altitude = str2float(tok.p, tok.end);
    r->fix.flags   |= GPS_LOCATION_HAS_ALTITUDE;
    /*
    * Hemisphere height
    */
    if(hem.p < hem.end) {
        r->fix.altitude += (double)str2float(hem.p, hem.end);
    }

    return 0;
}

static int nmea_reader_update_accuracy( NmeaReader*  r,
        Token        accuracy )
{
    Token   tok = accuracy;

    if (tok.p >= tok.end)
        return -1;

    //tok is cep*cc, we only want cep
    r->fix.accuracy = str2float(tok.p, tok.end);

    if (r->fix.accuracy == 99.99){
        return 0;
    }

    r->fix.flags   |= GPS_LOCATION_HAS_ACCURACY;
    return 0;
}

static int nmea_reader_update_bearing( NmeaReader*  r,
        Token        bearing )
{

    Token   tok = bearing;

    if (tok.p >= tok.end)
        return -1;

    r->fix.bearing  = str2float(tok.p, tok.end);
    r->fix.flags   |= GPS_LOCATION_HAS_BEARING;
    return 0;
}


static int nmea_reader_update_speed( NmeaReader*  r,
        Token        speed )
{
    Token   tok = speed;

    if (tok.p >= tok.end)
        return -1;

    r->fix.speed    = str2float(tok.p, tok.end);
    r->fix.speed   *= 0.514444;    // fix for Speed Unit form Knots to Meters per Second
    r->fix.flags   |= GPS_LOCATION_HAS_SPEED;
    return 0;
}

/*
 * $GPGGA,133456,0000.348,S,00000.348,E,2,07,1.2,10.7,M,48.8,M,7,0001*5A
 *  |                                                               |
 *  `---------------------------------------------------------------`-- xor value
 */
#define NMEA_MINLEN 12
int nmea_verify_packet(char *buf, int len)
{
    char sum = 0, crc;
    char *bufend = buf + len;

    if(len < NMEA_MINLEN) return -1;

    if(buf[0] != '$' || bufend[-3] != '*')
        return -1;

    buf++;
    len -= 4;   /* 3 + 1 */

    while(len--)
        sum ^= buf[len];

    crc = (char)strtoul(&bufend[-2], NULL, 16);

    return sum != crc;
}

static void nmea_reader_parse(NmeaReader* r)
{
    /* 
     *  we received a complete sentence in NmeaReader
     *  now parse it to generate a new GPS fix or SV information ...
     */
    NmeaTokenizer  tzer[1];
    Token          tok;
    int rtype;
    char *pbuf = r->in;
    int len = r->pos;
    //gps_log(LOG_INFO, "nmea %s", pbuf);

    /**********************************************
    N/A G1B1 COM1
    PN N/A
    SN N/A
    HWVer V0.0
    FWVer R3.1.0Build1979
    All rights reserved.
    **********************************************/
    if(strcasestr(pbuf, "HWVer") || strcasestr(pbuf, "FWVer"))
    {
        gps_log(LOG_INFO, "%s\n", pbuf);
        return;
    }

    if(strcasestr(pbuf, "Copyright")){
        gps_log(LOG_INFO, "%s\n", pbuf);
        return;
    }

    if (len < NMEA_MINLEN) {
        if(strcasestr(pbuf, "$OK") || strcasestr(pbuf, "$Fail")) {
           gps_log(LOG_INFO, "%s\n", pbuf);
        }
        //gps_log(LOG_INFO, "len < NMEA_MINLEN %s\n", pbuf);
        return;
    }

    // legal check
    if(nmea_verify_packet(pbuf, len - 2) != 0) {
        //gps_log(LOG_INFO, "%s\n", pbuf);
        return;
    }

    if ((umGpsCallbacks.nmea_cb != NULL) && (gps_hal_get_state() & GPS_HAL_STATE_STARTED)) {
        struct timespec tv;
        unsigned long long mytimems;
        clock_gettime(CLOCK_REALTIME, &tv);
        mytimems = ((unsigned long long)tv.tv_sec * CPU_TICKS_PER_SECOND) +
                ((unsigned long long)tv.tv_nsec / NANOSECONDS_IN_CPU_TICK);
        umGpsCallbacks.nmea_cb(mytimems, pbuf, len);
    }

    nmea_tokenizer_init(tzer, pbuf, pbuf + len);
#ifdef GPS_DEBUG_TOKEN
    {
        int  n;
        D("Found %d tokens", tzer->count);
        for (n = 0; n < tzer->count; n++) {
            Token  tok = nmea_tokenizer_get(tzer,n);
            D("%2d: '%.*s'", n, tok.end-tok.p, tok.p);
        }
    }
#endif

    tok = nmea_tokenizer_get(tzer, 0);

    if (tok.p + 5 > tok.end) {
        gps_log(1, "short sentences");/*  */
        return;
    }

    // check for RNSS type
    if(!memcmp(tok.p, "BD", 2) || !memcmp(tok.p, "GB", 2)) {
        rtype = GNSS_CONSTELLATION_BEIDOU;
   	} else if(!memcmp(tok.p, "GA", 2)) {
		rtype = GNSS_CONSTELLATION_GALILEO;
    } else
        rtype = GNSS_CONSTELLATION_GPS;
    // ignore first two characters.
    tok.p += 2;
    if(!memcmp(tok.p, "GGA", 3))
    {
         if(tzer->count == GGA_TOKEN_COUNT) {
            // GPS fix
            Token  tok_fixstaus      = nmea_tokenizer_get(tzer,6);
            /*
            * fix flag only valid from 0 to 7
            */
            if (tok_fixstaus.p[0] > '0' && tok_fixstaus.p[0] < '8') {

                Token  tok_time          = nmea_tokenizer_get(tzer,1);
                Token  tok_latitude      = nmea_tokenizer_get(tzer,2);
                Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer,3);
                Token  tok_longitude     = nmea_tokenizer_get(tzer,4);
                Token  tok_longitudeHemi = nmea_tokenizer_get(tzer,5);
                Token  tok_altitude      = nmea_tokenizer_get(tzer,9);
                Token  tok_altHemOffset = nmea_tokenizer_get(tzer,11);
                nmea_reader_update_time(r, tok_time);
                nmea_reader_update_latlong(r, tok_latitude,
                        tok_latitudeHemi.p[0],
                        tok_longitude,
                        tok_longitudeHemi.p[0]);
                nmea_reader_update_altitude(r, tok_altitude, tok_altHemOffset);
                if (r->fix.flags & GPS_LOCATION_HAS_LAT_LONG && r->fix.flags & GPS_LOCATION_HAS_ALTITUDE) {
                    gps_hal_update_location(r->fix);
                    r->fix.flags = 0;
                }
            }
        }
    } else if ( !memcmp(tok.p, "GLL", 3) ) {
        Token  tok_fixstaus      = nmea_tokenizer_get(tzer,6);

        if (tok_fixstaus.p[0] == 'A') {

            Token  tok_latitude      = nmea_tokenizer_get(tzer,1);
            Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer,2);
            Token  tok_longitude     = nmea_tokenizer_get(tzer,3);
            Token  tok_longitudeHemi = nmea_tokenizer_get(tzer,4);
            Token  tok_time          = nmea_tokenizer_get(tzer,5);
            nmea_reader_update_time(r, tok_time);
            nmea_reader_update_latlong(r, tok_latitude,
                    tok_latitudeHemi.p[0],
                    tok_longitude,
                    tok_longitudeHemi.p[0]);
        }
    } else if ( !memcmp(tok.p, "GSA", 3) ) {
        if(tzer->count == GSA_TOKEN_COUNT) {
            Token  tok_fixStatus   = nmea_tokenizer_get(tzer, 2);
            int i;
            Token tok_systemID = nmea_tokenizer_get(tzer, tzer->count-1);
            if( tok_systemID.p != tok_systemID.end) {
                if(tok_systemID.p[0] == '1' && tok_systemID.p[1] == '*')
                    rtype = GNSS_CONSTELLATION_GPS;
                else if(tok_systemID.p[0] == '3' && tok_systemID.p[1] == '*')
                    rtype = GNSS_CONSTELLATION_GALILEO;
                else if(tok_systemID.p[0] == '4' && tok_systemID.p[1] == '*')
                    rtype = GNSS_CONSTELLATION_BEIDOU;
            }

            if (tok_fixStatus.p[0] != '\0' && tok_fixStatus.p[0] != '1') {

                Token tok_accuracy      = nmea_tokenizer_get(tzer, 15);
                nmea_reader_update_accuracy(r, tok_accuracy);
                //
                // h30/h40/h41 GPS or BD only:
                //      $GPGSA,A,3,2,5,6,7,9,13,20,30,25,29,15,,1.445,0.924,1.111*0D
                // h40 hybrid:
                //      $GNGSA,A,3,166,168,169,171,173,174,161,162,163,164,165,167,0.688,0.837,0.554*24
                // h41 hybrid:
                //      $GNGSA,A,3,56,58,59,61,63,64,51,52,53,54,55,57,1.084,0.689,0.837*2E
                // h51 Hybrid:
                //      $GNGSA,A,3,27,01,07,08,09,11,16,10,30,28,,,1.12,0.72,0.86,1*05  ==> sysid=1, GPS
                //      $GNGSA,A,3,01,02,03,04,05,07,08,10,13,,,,1.12,0.72,0.86,4*00    ==> sysid=4, BDS
                // h51 GPS or BD only:
                //      $GPGSA,A,3,02,03,06,09,12,17,19,23,28,25,,,1.34,0.85,1.04,1*1E
                //      $GBGSA,A,3,02,03,06,09,12,17,19,23,28,25,,,1.34,0.85,1.04,1*1E

                for (i = 3; i <= 14; ++i) {
                    Token  tok_prn  = nmea_tokenizer_get(tzer, i);
                    int prn = str2int(tok_prn.p, tok_prn.end);

                    /* available for PRN 1-255 */
                    // TODO
                    // check PRN for BD2 satellites not starting from 160, or greater than 192
                    // check PRN for GLONASS or GALILEO satellites PRN numbering
                    if(prn > 0 && prn < MAX_SVS) {
                        // BDS maxium sv num :64
                        if(rtype == GNSS_CONSTELLATION_BEIDOU) {
                            if(prn < BDS_MAX_SVS) {               
                                prn += BDS_SV_OFFSET;
                            } else {
                                gps_log(LOG_ERROR, "abnormal BD SV info!");
                            }
                        }
                        else if(rtype == GNSS_CONSTELLATION_GALILEO ) {
                            if(prn < GAL_MAX_SVS) {               
                                prn += GAL_SV_OFFSET;
                            } else {
                                gps_log(LOG_ERROR, "abnormal GAL SV info!");
                            }
                        }
                        r->sv_list_tmp[prn].flags |= GNSS_SV_FLAGS_USED_IN_FIX;
                        r->sv_list_tmp[prn].svid = prn;

                        /* mark this parameter to identify the GSA is in fixed state */
                        r->gsa_fixed = 1;
                    }
                } // end for(;;)
            } else {
                if (r->gsa_fixed == 1) {
                    for(i = 0; i < MAX_SVS; i++) {
                        r->sv_list[i].flags &= ~GNSS_SV_FLAGS_USED_IN_FIX;
                        r->sv_list_tmp[i].flags &= ~GNSS_SV_FLAGS_USED_IN_FIX;
                    }

                    r->gsa_fixed = 0;
                    r->sv_status_commit = 1;
                }
            }
        }
    } else if ( !memcmp(tok.p, "GSV", 3) ) {
        Token  tok_noSatellites  = nmea_tokenizer_get(tzer, 3);
        int    noSatellites = str2int(tok_noSatellites.p, tok_noSatellites.end);

        Token  tok_noSentences   = nmea_tokenizer_get(tzer, 1);
        Token  tok_sentence      = nmea_tokenizer_get(tzer, 2);

        if (noSatellites > 0 && (tzer->count % 4) == 1) 
        {

            int sentence = str2int(tok_sentence.p, tok_sentence.end);
            int totalSentences = str2int(tok_noSentences.p, tok_noSentences.end);
            static int num_svs;
            int i;
            int prn;

            static int ff = 0;  // first found
            if (sentence == 1) {
                for(i = 0; i < MAX_SVS; i++) {
                    if(r->sv_list_tmp[i].constellation == rtype) r->sv_list_tmp[i].svid = 0;
                }
                num_svs = 0;
                ff = 1;
            } else {
                if(!ff || num_svs != (sentence-1)*4) {
                    gps_log(LOG_INFO, "partial GSV, skip.");
                    return;
                }
            }

            i = 0;
            while (i < ((tzer->count/4) - 1) && num_svs < noSatellites)
            {
                Token  tok_prn = nmea_tokenizer_get(tzer, i * 4 + 4);
                Token  tok_elevation = nmea_tokenizer_get(tzer, i * 4 + 5);
                Token  tok_azimuth = nmea_tokenizer_get(tzer, i * 4 + 6);
                Token  tok_snr = nmea_tokenizer_get(tzer, i * 4 + 7);

                prn = str2int(tok_prn.p, tok_prn.end);
                if(prn > 0 && prn < MAX_SVS) 
                {
                    if(rtype == GNSS_CONSTELLATION_BEIDOU) {
                        // * 124 ~ 187 BDS
                        if(prn < BDS_MAX_SVS) {
                            prn += BDS_SV_OFFSET;
                        } else {
                            gps_log(LOG_ERROR, "abnormal BDS SV info!");
                        }
                    }
                    else if(rtype == GNSS_CONSTELLATION_GALILEO) {
                        // * * 88~123 GAL
                        if(prn < GAL_MAX_SVS) {
                            prn += GAL_SV_OFFSET;
                        } else {
                            gps_log(LOG_ERROR, "abnormal GAL SV info!");
                        }
                    }
                    r->sv_list_tmp[prn].svid = prn;
                    r->sv_list_tmp[prn].elevation = str2float(tok_elevation.p, tok_elevation.end);
                    r->sv_list_tmp[prn].azimuth = str2float(tok_azimuth.p, tok_azimuth.end);
                    r->sv_list_tmp[prn].c_n0_dbhz = str2float(tok_snr.p, tok_snr.end);
                    r->sv_list_tmp[prn].constellation = rtype;

                    num_svs++;
                }
                i++;
            }

            if (sentence == totalSentences) {
                //memcpy(r->sv_status[rtype].sv_list, r->sv_status_tmp[rtype].sv_list, sizeof(GpsSvInfo)*GPS_MAX_SVS);
                //r->sv_status[rtype].num_svs = r->sv_status_tmp[rtype].num_svs;

                // $GPGSV,2,1,...
                // $GPGSV,2,2,... commit=1
                // $BDGSV,2,1,...
                // $BDGSV,2,2,... commit=0,changed=1
                // if no GSV, then trigger changed in RMC
                r->sv_status_commit = 1;
                if ( (r->nmea_mode != POS_MODE_GN)
                        || ((r->nmea_mode == POS_MODE_GN) && (rtype == GNSS_CONSTELLATION_BEIDOU)) ) {
                    memcpy(r->sv_list, r->sv_list_tmp, sizeof(r->sv_list));
                    r->sv_status_commit = 0;
                    r->sv_status_changed = 1;
                }

                num_svs = 0;
                ff = 0;
            }
        }

        if(r->sv_status_changed)
        {
            gps_hal_update_svstatus(r->sv_list);
        }
    } else if ( !memcmp(tok.p, "RMC", 3) ) { //NMEA first line
        // SEE ALSO "GSV" COMMENTS
        // EPOCH begin, if last epoch not sent out, mark it as changed
        if(tzer->count == RMC_TOKEN_COUNT) {
            if(r->sv_status_commit) {
                memcpy(r->sv_list, r->sv_list_tmp, sizeof(r->sv_list));
                r->sv_status_commit = 0;
                r->sv_status_changed = 1;
            }
            if(!memcmp(tok.p - 2, "GNR", 3)) {
                r->nmea_mode = POS_MODE_GN;
            } else if(!memcmp(tok.p - 2, "BDR", 3) || !memcmp(tok.p - 2, "GBR", 3)) {
                r->nmea_mode = POS_MODE_BD;
            } else {
                r->nmea_mode = POS_MODE_GPS;
            }

            Token  tok_fixStatus     = nmea_tokenizer_get(tzer,2);

            if (tok_fixStatus.p[0] == 'A')
            {
                Token  tok_time          = nmea_tokenizer_get(tzer,1);
                Token  tok_latitude      = nmea_tokenizer_get(tzer,3);
                Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer,4);
                Token  tok_longitude     = nmea_tokenizer_get(tzer,5);
                Token  tok_longitudeHemi = nmea_tokenizer_get(tzer,6);
                Token  tok_speed         = nmea_tokenizer_get(tzer,7);
                Token  tok_bearing       = nmea_tokenizer_get(tzer,8);
                Token  tok_date          = nmea_tokenizer_get(tzer,9);

                nmea_reader_update_date( r, tok_date, tok_time );
                nmea_reader_update_latlong( r, tok_latitude,
                        tok_latitudeHemi.p[0],
                        tok_longitude,
                        tok_longitudeHemi.p[0] );
                nmea_reader_update_bearing( r, tok_bearing );
                nmea_reader_update_speed( r, tok_speed );
                if (r->fix.flags & GPS_LOCATION_HAS_LAT_LONG && r->fix.flags & GPS_LOCATION_HAS_ALTITUDE) {
                    gps_hal_update_location(r->fix);
                    r->fix.flags = 0;
                }
            }
        }
        memset(r->sv_list_tmp, 0, sizeof(r->sv_list_tmp));    // clear the history sv info
    } else if ( !memcmp(tok.p, "VTG", 3) ) {
        Token  tok_fixStatus     = nmea_tokenizer_get(tzer,9);

        if (tok_fixStatus.p[0] != '\0' && tok_fixStatus.p[0] != 'N')
        {
            Token  tok_bearing       = nmea_tokenizer_get(tzer,1);
            Token  tok_speed         = nmea_tokenizer_get(tzer,5);
            nmea_reader_update_bearing( r, tok_bearing );
            nmea_reader_update_speed  ( r, tok_speed );
        }

    } else if ( !memcmp(tok.p, "ZDA", 3) ) {
        Token  tok_time;
        Token  tok_year  = nmea_tokenizer_get(tzer,4);

        if (tok_year.p[0] != '\0') {

            Token  tok_day   = nmea_tokenizer_get(tzer,2);
            Token  tok_mon   = nmea_tokenizer_get(tzer,3);
            nmea_reader_update_cdate( r, tok_day, tok_mon, tok_year );
        }

        tok_time  = nmea_tokenizer_get(tzer,1);

        if (tok_time.p[0] != '\0') {
            nmea_reader_update_time(r, tok_time);
        }

    } else {
        tok.p -= 2;
    }
}

static void nmea_reader_addc(NmeaReader *reader, char c)
{
    if(reader->overflow) {
        reader->overflow = (c != '\n');
        return;
    }

    if(reader->pos >= NMEA_MAX_SIZE - 1) {
        reader->overflow = 1;
        reader->pos = 0;
        return;
    }

    if(reader->pos == 0 && c != '$' ) {
        return;
    }

    reader->in[reader->pos++] = c;
    if(c == '\n') {
        reader->in[reader->pos] = '\0';
        nmea_reader_parse(reader);
        reader->pos = 0;
        memset(reader->in, 0x0, sizeof(reader->in));
    }
}

void um_gps_nmea_parse(char *buf, u32 len)
{
    NmeaReader *reader = &nmeaReader;
	gps_log(LOG_INFO, "enter %s, len = %d\n", __FUNCTION__, len);

    if(buf == NULL) {
        return;
    }

    for(int i = 0; i < len; i ++) {
        nmea_reader_addc(reader, buf[i]);
    }
	gps_log(LOG_INFO, "leave %s", __FUNCTION__);
}
