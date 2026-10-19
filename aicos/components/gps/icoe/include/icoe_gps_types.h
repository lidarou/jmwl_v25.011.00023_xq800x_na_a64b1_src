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

/**
 * System basic data types
 */

#ifndef _GPS_TYPES_H_
#define _GPS_TYPES_H_

#include <pthread.h>
#include <aic_type.h>

#if 0
struct tm {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;

  int tm_isdst;
};
#endif

#define GNSS_MAX_SVS 128

typedef uint16_t AGpsType;
typedef uint16_t ApnIpType;

enum {
    GNSS_CONSTELLATION_UNKNOWN = 0,
    GNSS_CONSTELLATION_GPS = 1,
    GNSS_CONSTELLATION_SBAS = 2,
    GNSS_CONSTELLATION_GLONASS = 3,
    GNSS_CONSTELLATION_QZSS = 4,
    GNSS_CONSTELLATION_BEIDOU = 5,
    GNSS_CONSTELLATION_GALILEO = 6,
};

enum {
    GPS_STATUS_NONE = 0,
    GPS_STATUS_SESSION_BEGIN = 1,
    GPS_STATUS_SESSION_END = 2,
    GPS_STATUS_ENGINE_ON = 3,
    GPS_STATUS_ENGINE_OFF = 4,
};

typedef struct {
    /** set to sizeof(GpsStatus) */
    size_t          size;
    uint16_t        status;
} GpsStatus;

/*
 * Milliseconds since January 1, 1970
 */
typedef int64_t GpsUtcTime;
/*
 * Represents a location. 
 */
typedef struct {
/*
 * set to sizeof(GpsLocation) 
 */
size_t          size;
/*
 * Contains GpsLocationFlags bits. 
 */
uint16_t        flags;
/*
 * Represents latitude in degrees. 
 */
double          latitude;
/*
 * Represents longitude in degrees.
 */
double          longitude;
/**
  * Represents altitude in meters above the WGS 84 reference ellipsoid.     
  */
double          altitude;
/** Represents speed in meters per second. */
float           speed;
/** Represents heading in degrees. */
float           bearing;
/** Represents expected accuracy in meters. */
float           accuracy;
/** Timestamp for the location fix. */
GpsUtcTime      timestamp;
} GpsLocation;


/** * Constellation type of GnssSvInfo */
typedef uint8_t                         GnssConstellationType;
typedef uint8_t                                 GnssSvFlags;


typedef struct {    
    /** set to sizeof(GnssSvInfo) */ 
    size_t size;
/**
* Pseudo-random number for the SV, or FCN/OSN number for Glonass. The
* distinction is made by looking at constellation field. Values should be
* in the range of:     
*     
* - GPS:     1-32     
* - SBAS:    120-151, 183-192     
* - GLONASS: 1-24, the orbital slot number (OSN), if known.  Or, if not:     
*            93-106, the frequency channel number (FCN) (-7 to +6) offset by + 100     
*            i.e. report an FCN of -7 as 93, FCN of 0 as 100, and FCN of +6 as 106.     
* - QZSS:    193-200     
* - Galileo: 1-36     
* - Beidou:  1-37     */    
int16_t svid;    
/**     
* Defines the constellation of the given SV. Value should be one of those
* GNSS_CONSTELLATION_* constants     
*/    
GnssConstellationType constellation;    
/**     * Carrier-to-noise density in dB-Hz, typically in the range [0, 63].
* It contains the measured C/N0 value for the signal at the antenna port.     
*     * This is a mandatory value.     
*/
float c_n0_dbhz;
/** Elevation of SV in degrees. */
float elevation;
/** Azimuth of SV in degrees. */
float azimuth;
/**     * Contains additional data about the given SV. Value should be one of those     * GNSS_SV_FLAGS_* constants     */
GnssSvFlags flags;
} GnssSvInfo;

/**
 * Represents SV status.
 */
typedef struct {
    /** set to sizeof(GnssSvStatus) */
    size_t size;

    /** Number of GPS SVs currently visible, refers to the SVs stored in sv_list */
    int num_svs;
    /**
     * Pointer to an array of SVs information for all GNSS constellations,
     * except GPS, which is reported using sv_list
     */
    GnssSvInfo gnss_sv_list[GNSS_MAX_SVS];

} GnssSvStatus;

/**
 * Callback with location information. Can only be called from a thread created
 * by create_thread_cb.
 */
typedef void (* gps_location_callback)(GpsLocation* location);

/**
 * Callback with status information. Can only be called from a thread created by
 * create_thread_cb.
 */
typedef void (* gps_status_callback)(GpsStatus* status);

/**
 * Callback with SV status information.
 * Can only be called from a thread created by create_thread_cb.
 */
typedef void (* gnss_sv_status_callback)(GnssSvStatus* sv_info);
/**
 * Callback for reporting NMEA sentences. Can only be called from a thread
 * created by create_thread_cb.
 */
typedef void (* gps_nmea_callback)(GpsUtcTime timestamp, const char* nmea, int length);

typedef struct {
    /** set to sizeof(GpsCallbacks) */
    size_t      size;
    gps_location_callback location_cb;
    gps_status_callback status_cb;
    gnss_sv_status_callback gnss_sv_status_cb;
    gps_nmea_callback nmea_cb;
} GpsCallbacks;

typedef float              FLT;                /* 4 bytes floating point   */
typedef double             DBL;                /* 8 bytes floating point   */
	
#define GPS_NMEA_STRING_MAXLEN 1024

#define GPS_MAX_SVS 32
#define SBAS_MAX_SVS 32
#define GLN_MAX_SVS 24
#define GAL_MAX_SVS 36
#define RESV_SVS_NUM 4
#define BDS_MAX_SVS 64
#define QZSS_SVS_NUM 12

#define BDS_SV_OFFSET (GPS_MAX_SVS + SBAS_MAX_SVS + GLN_MAX_SVS + GAL_MAX_SVS)
#define GAL_SV_OFFSET (GPS_MAX_SVS + SBAS_MAX_SVS + GLN_MAX_SVS)
/*
* 1~32 GPS
* 33~63 SBAS
* 64~87 GLO
* 88~123 GAL
* 124 ~ 187 BDS
* 192 ~ 202 QZSS
*/
#define  MAX_SVS (GPS_MAX_SVS + SBAS_MAX_SVS + GLN_MAX_SVS + GAL_MAX_SVS + RESV_SVS_NUM + BDS_MAX_SVS + QZSS_SVS_NUM)
#define  NMEA_MAX_SIZE  256

#define PARAMETER_NOT_USED(p) ((void)(p))

#ifndef void
#define  void  void               /* void data type   */
#endif

/* Macros */
#ifndef NULL
#define NULL              ((void *)0)          /* NULL pointer */
#endif

#ifndef TRUE
#define TRUE              (1)
#endif

#ifndef FALSE
#define FALSE             (0)
#endif

/* maximum & minimum value macros */
#ifndef MAX
#define MAX(a, b)       ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b)       ((a) < (b) ? (a) : (b))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

#endif /* _GPS_TYPES_H_ */

