//---------------------------------------------------------------------------

#ifndef NMEAPackageH
#define NMEAPackageH

#include <stdio.h>
#include "MTKPackage.h"

#define NEMA_VERSION_2.3_LATER

#define NUM_OF_NMEA     		6
#define GPS_ID_MAX        		32
#define SBAS_ID_MIN             33
#define SBAS_ID_MAX             54
#define GPS_SUP_ID_MIN          193
#define GPS_SUP_ID_MAX          200
#define GLO_ID_MIN        		65
#define GLO_ID_MAX              96
#define KM_TO_MILE				0.621371192
#define MPH_TO_KNOT             0.868976242
#define DEGREE_TO_MINUTE        60
#define MAX_SATELLITE_NUM       24
#define SATELLITE_NUM           4      // 每個 GSV sentence 最多包含量

//---------------------------------------------------------------------------
enum NmeaSatId
{
	NMEA_GPS = 				    1,
	NMEA_SBAS,
	NMEA_NOT_USE,
	NMEA_GLONASS,
	NMEA_QZSS,
	NMEA_BEIDOU
} ;

enum NmeaEvent
{
	NMEA_GGA_OFFSET,
	NMEA_GLL_OFFSET,
	NMEA_GSA_OFFSET,
	NMEA_GSV_OFFSET,
	NMEA_RMC_OFFSET,
	NMEA_VTG_OFFSET,

	NMEA_GGA =          (1 << NMEA_GGA_OFFSET),
	NMEA_GLL = 			(1 << NMEA_GLL_OFFSET),
	NMEA_GSA = 			(1 << NMEA_GSA_OFFSET),
	NMEA_GSV =        	(1 << NMEA_GSV_OFFSET),
	NMEA_RMC =          (1 << NMEA_RMC_OFFSET),
	NMEA_VTG =          (1 << NMEA_VTG_OFFSET)
} ;

//---------------------------------------------------------------------------
typedef struct GLLInfo
{
	char dataStatus ;
#ifdef NEMA_VERSION_2.3_LATER
	char mode ;
	char reserve[2] ;
#else
	char reserve[3] ;
#endif
	unsigned int utc ;
	double latitude ;
	double longitude ;
} GLLInfo ;

typedef struct RMCInfo
{
	char dataStatus ;
	char magneticVariationUnit ;
#ifdef NEMA_VERSION_2.3_LATER
	char mode ;
	char reserve[2] ;
#else
	char reserve[3] ;
#endif
	unsigned int utc ;
	unsigned int date ;
	float speed ;
	float trackAngle ;
	float magneticVariation ;
	double latitude ;
	double longitude ;
} RMCInfo ;

typedef struct GSVSatelliteInfo
{
	unsigned int prnNumber ;
	int elevation ;
	unsigned int azimuth ;
	int snr ;
} GSVSatelliteInfo ;

typedef struct GSVInfo
{
	unsigned int totalSentence ;
	unsigned int nowSentence ;
	unsigned int totalSatellite ;
	GSVSatelliteInfo satelliteInfo[SATELLITE_NUM] ;
} GSVInfo ;

typedef struct GSAInfo
{
	char fixAction ;
	char reserve[3] ;
	unsigned int fixStatus ;
	unsigned int numOfSatellite ;
	unsigned int prnNumber[MAX_SATELLITE_NUM] ;
	float pdop ;
	float hdop ;
	float vdop ;
} GSAInfo ;

typedef struct VTGInfo
{
	char trueCourseUnit ;
	char magneticCourseUnit ;
	char speedKnotsUnit ;
	char speedKilometersUnit ;
#ifdef NEMA_VERSION_2.3_LATER
	char mode ;
	char reserve[3] ;
#endif
	float trueCourse ;
	float magneticCourse ;
	float speedKnots ;
	float speedKilometers ;
} VTGInfo ;

typedef struct GGAInfo
{
	char altitudeUnit ;
	char heightOfGeoidUnit ;
	char reserve[2] ;
	unsigned int utc ;
	unsigned int fixQuality ;
	unsigned int numOfSatellites ;
	unsigned int dgpsId ;
	float horizontalDilution ;
	float altitude ;
	float heightOfGeoid ;
	float updateInterval ;
	double latitude ;
	double longitude ;
} GGAInfo ;

typedef union NmeaTypeInfo
{
	GLLInfo gll ;
	RMCInfo rmc ;
	GSVInfo gsv ;
	GSAInfo gsa ;
	VTGInfo vtg ;
	GGAInfo gga ;
} NmeaTypeInfo ;

typedef struct NmeaInfo
{
	int id ;
	NmeaTypeInfo type ;
} NmeaInfo ;

typedef int (*MtkPktToNmeaFun)(FILE *, MTKLog) ;

//---------------------------------------------------------------------------
int MtkPktToNmea(FILE *fp, MTKLog log, int event) ;
int inline MtkPktToNmeaGLL(FILE *fp, MTKLog log) ;
int inline MtkPktToNmeaRMC(FILE *fp, MTKLog log) ;
int inline MtkPktToNmeaGSV(FILE *fp, MTKLog log) ;
int inline MtkPktToNmeaGSA(FILE *fp, MTKLog log) ;
int inline MtkPktToNmeaVTG(FILE *fp, MTKLog log) ;
int inline MtkPktToNmeaGGA(FILE *fp, MTKLog log) ;

int AnalysisNMEAData(unsigned char *buf, int bufSize, NmeaInfo *nmeaInfo) ;
int AnalysisRMCData(unsigned char *buf, int bufSize, NmeaInfo *nmeaInfo) ;
int AnalysisGSAData(unsigned char *buf, int bufSize, NmeaInfo *nmeaInfo) ;
int AnalysisGSVData(unsigned char *buf, int bufSize, NmeaInfo *nmeaInfo) ;
int JudgeNMEAPkt(WriteComInfo *comInfo, void *ackInfo, FILE *dwFp, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;

int inline SatIdSort(const void *element1, const void *element2) ;
double inline DegreeToMinute(double degree) ;

#endif
